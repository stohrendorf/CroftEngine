#include "audiostreamdecoder.h"

#include "ffmpeg/avframeptr.h"
#include "ffmpeg/stream.h"
#include "ffmpeg/util.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cerrno>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
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
    : fmtContext{fmtContext}
    , stream{std::make_unique<ffmpeg::Stream>(fmtContext, AVMEDIA_TYPE_AUDIO, rplFakeAudioHack)}
    , swrContext{swr_alloc_set_opts(nullptr,
                                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                    stream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_S16,
                                    stream->context->sample_rate,
                                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                    stream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                    stream->context->sample_fmt,
                                    stream->context->sample_rate,
                                    0,
                                    nullptr)}
{
  gsl_Expects(stream->context->channels == 1 || stream->context->channels == 2);

  if(swrContext == nullptr)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate resampler context"));
  }

  if(swr_init(swrContext) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize the resampling context"));
  }
}

AudioStreamDecoder::~AudioStreamDecoder()
{
  swr_free(&swrContext);
}

bool AudioStreamDecoder::push(const AVPacket& packet)
{
  const std::unique_lock lock{mutex};

  if(packet.stream_index != stream->index || queue.size() >= QueueLimit)
    return false;
  lastPacketPts = packet.pts;

  if(const auto err = avcodec_send_packet(stream->context, &packet); err != 0)
  {
    handleSendPacketError(err, stream->context);
  }

  int err;
  while((err = avcodec_receive_frame(stream->context, audioFrame.frame)) == 0)
  {
    const auto outSamples = swr_get_out_samples(swrContext, audioFrame.frame->nb_samples);
    if(outSamples < 0)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to receive resampled audio data";
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to receive resampled audio data"));
    }

    std::vector<int16_t> audio(gsl::narrow_cast<size_t>(outSamples * getChannels()), 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* audioData = reinterpret_cast<uint8_t*>(audio.data());

    const auto framesDecoded = swr_convert(swrContext,
                                           &audioData,
                                           outSamples,
                                           // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                                           const_cast<const uint8_t**>(audioFrame.frame->data),
                                           audioFrame.frame->nb_samples);
    if(framesDecoded < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Error while converting"));
    }

    // cppcheck-suppress invalidFunctionArg
    audio.resize(gsl::narrow_cast<size_t>(framesDecoded * getChannels()));

    queue.push(std::move(audio));
  }
  if(err != AVERROR(EAGAIN))
    BOOST_LOG_TRIVIAL(info) << "Audio stream chunk decoded: " << getAvError(err);

  return true;
}

size_t AudioStreamDecoder::read(int16_t* buffer, size_t bufferSize)
{
  const std::unique_lock lock{mutex};

  size_t written = 0;
  while(bufferSize != 0 && !queue.empty())
  {
    auto& src = queue.front();
    const auto frames = std::min(bufferSize, src.size() / getChannels());

    gsl_Assert(bufferSize >= frames);

    std::copy_n(src.data(), getChannels() * frames, buffer);
    bufferSize -= frames;
    written += frames;
    buffer += getChannels() * frames;

    src.erase(src.begin(), std::next(src.begin(), getChannels() * frames));
    if(src.empty())
    {
      queue.pop();
    }
  }

  std::fill_n(buffer, getChannels() * bufferSize, int16_t{0});
  return written;
}

audio::Clock::duration AudioStreamDecoder::getDuration() const
{
  return ffmpeg::toDuration<audio::Clock::duration>(stream->stream->duration, stream->stream->time_base);
}

int AudioStreamDecoder::getSampleRate() const noexcept
{
  return stream->context->sample_rate;
}

std::chrono::milliseconds AudioStreamDecoder::getPosition() const
{
  const std::unique_lock lock{mutex};
  return ffmpeg::toDuration<std::chrono::milliseconds>(lastPacketPts, stream->stream->time_base);
}

void AudioStreamDecoder::seek(const std::chrono::milliseconds& position)
{
  const std::unique_lock lock{mutex};
  const auto ts = ffmpeg::fromDuration(position, stream->stream->time_base);
  if(av_seek_frame(fmtContext, stream->index, ts, 0) < 0)
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to seek audio stream to " << position.count() << "ms";
  }
  queue = {};
}

int AudioStreamDecoder::getChannels() const noexcept
{
  return stream->context->channels;
}
} // namespace audio
