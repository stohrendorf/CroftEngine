#include "audiostreamdecoder.h"

#include "core.h"
#include "ffmpeg/avframeptr.h"
#include "ffmpeg/stream.h"
#include "ffmpeg/util.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/samplefmt.h>
#include <libavutil/version.h>
#include <libswresample/swresample.h>
}

namespace audio
{
namespace
{
std::string getAvError(int err)
{
  std::vector<char> tmp(1024, 0);
  if(av_strerror(err, tmp.data(), tmp.size()) < 0)
    return "Unknown error " + std::to_string(err);

  return tmp.data();
}

void handleSendPacketError(int err, AVCodecContext* ctx)
{
  if(err == AVERROR(EINVAL))
  {
    BOOST_LOG_TRIVIAL(info) << "Flushing audio decoder";
    avcodec_flush_buffers(ctx);
  }
  else
  {
    switch(err)
    {
    case AVERROR(EAGAIN):
      BOOST_LOG_TRIVIAL(error) << "Frames still present in audio decoder";
      break;
    case AVERROR(ENOMEM):
      BOOST_LOG_TRIVIAL(error) << "Failed to add packet to audio decoder queue";
      break;
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
    case AVERROR_EOF:
      BOOST_LOG_TRIVIAL(error) << "Audio decoder already flushed";
      break;
    default:
      break;
    }

    BOOST_LOG_TRIVIAL(error) << "Failed to send packet to audio decoder: " << getAvError(err);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send packet to audio decoder"));
  }
}
} // namespace

AudioStreamDecoder::AudioStreamDecoder(AVFormatContext* fmtContext, bool rplFakeAudioHack)
    : m_fmtContext{fmtContext}
    , m_stream{std::make_unique<ffmpeg::Stream>(fmtContext, AVMEDIA_TYPE_AUDIO, rplFakeAudioHack)}
#if LIBAVUTIL_VERSION_MAJOR < 58
    , m_swrContext{swr_alloc_set_opts(nullptr,
                                      // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                      m_stream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                      AV_SAMPLE_FMT_S16,
                                      m_stream->context->sample_rate,
                                      // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                      m_stream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                      m_stream->context->sample_fmt,
                                      m_stream->context->sample_rate,
                                      0,
                                      nullptr)}
#endif
{
#if LIBAVUTIL_VERSION_MAJOR >= 58
  gsl_Expects(m_stream->context->ch_layout.nb_channels == 1 || m_stream->context->ch_layout.nb_channels == 2);

  auto channelLayout = m_stream->context->ch_layout.nb_channels == 1 ? AVChannelLayout AV_CHANNEL_LAYOUT_MONO
                                                                     : AVChannelLayout AV_CHANNEL_LAYOUT_STEREO;
  gsl_Assert(swr_alloc_set_opts2(&m_swrContext,
                                 &channelLayout,
                                 AV_SAMPLE_FMT_S16,
                                 m_stream->context->sample_rate,
                                 &channelLayout,
                                 m_stream->context->sample_fmt,
                                 m_stream->context->sample_rate,
                                 0,
                                 nullptr)
             == 0);
#else
  gsl_Expects(m_stream->context->channels == 1 || m_stream->context->channels == 2);
#endif

  if(m_swrContext == nullptr)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate resampler context"));
  }

  if(swr_init(m_swrContext) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize the resampling context"));
  }
}

AudioStreamDecoder::~AudioStreamDecoder()
{
  swr_free(&m_swrContext);
}

bool AudioStreamDecoder::push(const AVPacket& packet)
{
  const std::unique_lock lock{m_mutex};

  if(packet.stream_index != m_stream->index || m_queue.size() >= QueueLimit)
    return false;
  m_lastPacketPts = packet.pts;

  if(const auto err = avcodec_send_packet(m_stream->context, &packet); err != 0)
  {
    handleSendPacketError(err, m_stream->context);
  }

  int err;
  while((err = avcodec_receive_frame(m_stream->context, m_audioFrame.frame)) == 0)
  {
    const auto outSamples = swr_get_out_samples(m_swrContext, m_audioFrame.frame->nb_samples);
    if(outSamples < 0)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to receive resampled audio data";
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to receive resampled audio data"));
    }

    std::vector<int16_t> audio(gsl::narrow_cast<size_t>(outSamples * getChannels()), 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* audioData = reinterpret_cast<uint8_t*>(audio.data());

    const auto framesDecoded = swr_convert(m_swrContext,
                                           &audioData,
                                           outSamples,
                                           // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                                           const_cast<const uint8_t**>(m_audioFrame.frame->data),
                                           m_audioFrame.frame->nb_samples);
    if(framesDecoded < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Error while converting"));
    }

    // cppcheck-suppress invalidFunctionArg
    audio.resize(gsl::narrow_cast<size_t>(framesDecoded * getChannels()));

    m_queue.push(std::move(audio));
  }
  if(err != AVERROR(EAGAIN))
    BOOST_LOG_TRIVIAL(info) << "Audio stream chunk decoded: " << getAvError(err);

  return true;
}

size_t AudioStreamDecoder::read(int16_t* buffer, size_t bufferSize)
{
  const std::unique_lock lock{m_mutex};

  size_t written = 0;
  while(bufferSize != 0 && !m_queue.empty())
  {
    auto& src = m_queue.front();
    const auto frames = std::min(bufferSize, src.size() / getChannels());

    gsl_Assert(bufferSize >= frames);

    std::copy_n(src.data(), getChannels() * frames, buffer);
    bufferSize -= frames;
    written += frames;
    buffer += getChannels() * frames;

    src.erase(src.begin(), std::next(src.begin(), getChannels() * frames));
    if(src.empty())
    {
      m_queue.pop();
    }
  }

  std::fill_n(buffer, getChannels() * bufferSize, int16_t{0});
  return written;
}

Clock::duration AudioStreamDecoder::getDuration() const
{
  return ffmpeg::toDuration<Clock::duration>(m_stream->stream->duration, m_stream->stream->time_base);
}

int AudioStreamDecoder::getSampleRate() const noexcept
{
  return m_stream->context->sample_rate;
}

std::chrono::milliseconds AudioStreamDecoder::getPosition() const
{
  const std::unique_lock lock{m_mutex};
  return ffmpeg::toDuration<std::chrono::milliseconds>(m_lastPacketPts, m_stream->stream->time_base);
}

void AudioStreamDecoder::seek(const std::chrono::milliseconds& position)
{
  const std::unique_lock lock{m_mutex};
  const auto ts = ffmpeg::fromDuration(position, m_stream->stream->time_base);
  if(av_seek_frame(m_fmtContext, m_stream->index, ts, 0) < 0)
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to seek audio stream to " << position.count() << "ms";
  }
  m_queue = {};
}

int AudioStreamDecoder::getChannels() const noexcept
{
#if LIBAVUTIL_VERSION_MAJOR >= 58
  return m_stream->context->ch_layout.nb_channels;
#else
  return m_stream->context->channels;
#endif
}
} // namespace audio
