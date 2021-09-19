#include "adecoder.h"

#include "avframeptr.h"
#include "stream.h"
#include "util.h"

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

namespace video
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
} // namespace

ADecoder::ADecoder(const std::filesystem::path& filename)
{
  if(avformat_open_input(&fmtContext, filename.string().c_str(), nullptr, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not open source file"));
  }

  if(avformat_find_stream_info(fmtContext, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream information"));
  }

  audioStream = std::make_unique<Stream>(fmtContext, AVMEDIA_TYPE_AUDIO, false);

#ifndef NDEBUG
  av_dump_format(fmtContext, 0, filename.string().c_str(), 0);
#endif

  Expects(audioStream->context->channels == 1 || audioStream->context->channels == 2);

  swrContext = swr_alloc_set_opts(nullptr,
                                  // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                  audioStream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                  AV_SAMPLE_FMT_S16,
                                  audioStream->context->sample_rate,
                                  // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                  audioStream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                  audioStream->context->sample_fmt,
                                  audioStream->context->sample_rate,
                                  0,
                                  nullptr);
  if(swrContext == nullptr)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate resampler context"));
  }

  if(swr_init(swrContext) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize the resampling context"));
  }

  Expects(av_new_packet(&packet, 0) == 0);
  fillQueues(false);
}

ADecoder::~ADecoder()
{
  swr_free(&swrContext);
  avformat_close_input(&fmtContext);
}

void ADecoder::fillQueues(bool looping)
{
  while(audioQueue.size() < QueueLimit)
  {
    const auto err = av_read_frame(fmtContext, &packet);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(err == AVERROR_EOF && looping)
    {
      Expects(av_seek_frame(fmtContext, audioStream->index, 0, 0) >= 0);
      continue;
    }
    else if(err != 0)
    {
      break;
    }

    decodePacket();
    av_packet_unref(&packet);
  }
}

void ADecoder::decodePacket()
{
  if(packet.stream_index != audioStream->index)
    return;

  if(const auto err = avcodec_send_packet(audioStream->context, &packet))
  {
    if(err == AVERROR(EINVAL))
    {
      BOOST_LOG_TRIVIAL(info) << "Flushing audio decoder";
      avcodec_flush_buffers(audioStream->context);
    }
    else
    {
      if(err == AVERROR(EAGAIN))
        BOOST_LOG_TRIVIAL(error) << "Frames still present in audio decoder";
      else if(err == AVERROR(ENOMEM))
        BOOST_LOG_TRIVIAL(error) << "Failed to add packet to audio decoder queue";
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      else if(err == AVERROR_EOF)
        BOOST_LOG_TRIVIAL(error) << "Audio decoder already flushed";

      BOOST_LOG_TRIVIAL(error) << "Failed to send packet to audio decoder: " << getAvError(err);
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send packet to audio decoder"));
    }
  }

  int err;
  while((err = avcodec_receive_frame(audioStream->context, audioFrame.frame)) == 0)
  {
    const auto outSamples = swr_get_out_samples(swrContext, audioFrame.frame->nb_samples);
    if(outSamples < 0)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to receive resampled audio data";
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to receive resampled audio data"));
    }

    std::vector<int16_t> audio(gsl::narrow_cast<size_t>(outSamples) * 2u, 0);
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
    audio.resize(gsl::narrow_cast<size_t>(framesDecoded) * 2u);

    audioQueue.push(std::move(audio));
  }
  if(err != AVERROR(EAGAIN))
    BOOST_LOG_TRIVIAL(info) << "Audio stream chunk decoded: " << getAvError(err);
}

size_t ADecoder::readStereo(int16_t* buffer, size_t bufferSize, bool looping)
{
  fillQueues(looping);

  size_t written = 0;
  while(bufferSize != 0 && !audioQueue.empty())
  {
    auto& src = audioQueue.front();
    const auto frames = std::min(static_cast<size_t>(bufferSize), src.size() / 2);

    Expects(bufferSize >= frames);

    std::copy_n(src.data(), 2 * frames, buffer);
    bufferSize -= frames;
    written += frames;
    buffer += 2 * frames;

    src.erase(src.begin(), std::next(src.begin(), 2 * frames));
    if(src.empty())
    {
      audioQueue.pop();
    }
  }

  std::fill_n(buffer, 2 * bufferSize, int16_t{0});
  return written;
}

audio::Clock::duration ADecoder::getDuration() const
{
  return toDuration<audio::Clock::duration>(audioStream->stream->duration, audioStream->stream->time_base);
}

int ADecoder::getSampleRate() const
{
  return audioStream->context->sample_rate;
}

std::chrono::milliseconds ADecoder::getPosition() const
{
  return toDuration<std::chrono::milliseconds>(audioStream->stream->cur_dts, audioStream->stream->time_base);
}

void ADecoder::seek(const std::chrono::milliseconds& position)
{
  const auto ts = fromDuration(position, audioStream->stream->time_base);

  Expects(av_seek_frame(fmtContext, audioStream->index, ts, 0) >= 0);
  audioQueue = {};
  fillQueues(false);
}
} // namespace video
