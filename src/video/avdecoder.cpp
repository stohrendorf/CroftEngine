#include "avdecoder.h"

#include "avframeptr.h"
#include "filtergraph.h"
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
#include <thread>
#include <utility>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
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

AVDecoder::AVDecoder(const std::string& filename)
{
  if(avformat_open_input(&fmtContext, filename.c_str(), nullptr, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not open source file"));
  }

  if(avformat_find_stream_info(fmtContext, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream information"));
  }

  videoStream = std::make_unique<Stream>(fmtContext, AVMEDIA_TYPE_VIDEO, false);

  audioStream = std::make_unique<Stream>(fmtContext, AVMEDIA_TYPE_AUDIO, true);

#ifndef NDEBUG
  av_dump_format(fmtContext, 0, filename.c_str(), 0);
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

  filterGraph.init(*videoStream);

  Expects(videoStream->stream->time_base.den != 0);
  audioFrameSize
    = audioStream->context->sample_rate * videoStream->stream->time_base.num / videoStream->stream->time_base.den;

  Expects(av_new_packet(&packet, 0) == 0);
  fillQueues();
}

AVDecoder::~AVDecoder()
{
  swr_free(&swrContext);
  avformat_close_input(&fmtContext);
}

void AVDecoder::fillQueues()
{
  {
    std::unique_lock lock(imgQueueMutex);
    if(audioQueue.size() >= QueueLimit || imgQueue.size() >= QueueLimit)
    {
      return;
    }
  }

  int err;
  while((err = av_read_frame(fmtContext, &packet)) == 0)
  {
    decodePacket();
    av_packet_unref(&packet);

    std::unique_lock lock(imgQueueMutex);
    if(audioQueue.size() >= QueueLimit || imgQueue.size() >= QueueLimit)
    {
      break;
    }
  }
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if(err != 0 && err != AVERROR_EOF)
  {
    BOOST_LOG_TRIVIAL(warning) << "fillQueues done: " << getAvError(err) << "; audio=" << audioQueue.size()
                               << ", video=" << imgQueue.size();
  }
}

std::optional<AVFramePtr> AVDecoder::takeFrame()
{
  std::unique_lock lock{imgQueueMutex};
  while(!frameReady)
    frameReadyCondition.wait(lock);
  frameReady = false;

  std::optional<AVFramePtr> img;
  if(!imgQueue.empty())
  {
    img = std::move(imgQueue.front());
    imgQueue.pop();
    const auto& tb = videoStream->stream->time_base;
    const auto audioTs = static_cast<double>(totalAudioFrames) / static_cast<double>(audioFrameSize);
    const auto videoTs = static_cast<double>(img->frame->pts) * tb.num / tb.den;
    if(audioTs < videoTs)
    {
      lock.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds{gsl::narrow_cast<int>((videoTs - audioTs) * 1000)});
      lock.lock();
    }
  }

  stopped = imgQueue.empty() && audioQueue.empty();

  return img;
}

void AVDecoder::decodeVideoPacket()
{
  if(const auto sendPacketErr = avcodec_send_packet(videoStream->context, &packet))
  {
    if(sendPacketErr == AVERROR(EINVAL))
    {
      BOOST_LOG_TRIVIAL(info) << "Flushing video decoder";
      avcodec_flush_buffers(videoStream->context);
    }
    else
    {
      if(sendPacketErr == AVERROR(EAGAIN))
        BOOST_LOG_TRIVIAL(error) << "Frames still present in video decoder";
      else if(sendPacketErr == AVERROR(ENOMEM))
        BOOST_LOG_TRIVIAL(error) << "Failed to add packet to video decoder queue";
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      else if(sendPacketErr == AVERROR_EOF)
        BOOST_LOG_TRIVIAL(error) << "Video decoder already flushed";

      BOOST_LOG_TRIVIAL(error) << "Failed to send packet to video decoder: " << getAvError(sendPacketErr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send packet to video decoder"));
    }
  }

  AVFramePtr videoFrame;
  int err;
  while((err = avcodec_receive_frame(videoStream->context, videoFrame.frame)) == 0)
  {
    if(const auto addFrameErr = av_buffersrc_add_frame(filterGraph.input, videoFrame.release()))
    {
      BOOST_LOG_TRIVIAL(error) << "Error while feeding the filtergraph: " << getAvError(addFrameErr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Error while feeding the filtergraph"));
    }
    videoFrame = AVFramePtr();

    while(true)
    {
      AVFramePtr filteredFrame;
      const auto ret = av_buffersink_get_frame(filterGraph.output, filteredFrame.frame);
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;
      if(ret < 0)
      {
        BOOST_LOG_TRIVIAL(error) << "Filter error: " << getAvError(ret);
        BOOST_THROW_EXCEPTION(std::runtime_error("Filter error"));
      }

      std::unique_lock lock(imgQueueMutex);
      imgQueue.push(std::move(filteredFrame));
    }
  }
  if(err != AVERROR(EAGAIN))
    BOOST_LOG_TRIVIAL(info) << "Video stream chunk decoded: " << getAvError(err);
}

void AVDecoder::decodeAudioPacket()
{
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

void AVDecoder::decodePacket()
{
  if(packet.stream_index == videoStream->index)
  {
    decodeVideoPacket();
  }
  else if(packet.stream_index == audioStream->index)
  {
    decodeAudioPacket();
  }
}

size_t AVDecoder::readStereo(int16_t* buffer, size_t bufferSize, bool)
{
  fillQueues();

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

  if(written == 0)
  {
    std::unique_lock lock{imgQueueMutex};
    if(!imgQueue.empty())
    {
      // audio ended prematurely - pad with zero audio data until all video frames are consumed
      written = bufferSize;
    }
  }

  totalAudioFrames += written;
  audioFrameOffset += written;
  Expects(audioFrameSize > 0);
  while(audioFrameOffset >= audioFrameSize)
  {
    audioFrameOffset -= audioFrameSize;
    std::unique_lock lock{imgQueueMutex};
    frameReady = true;
    frameReadyCondition.notify_one();
  }

  if(written == 0)
  {
    std::unique_lock lock{imgQueueMutex};
    frameReady = true;
    frameReadyCondition.notify_one();
  }

  std::fill_n(buffer, 2 * bufferSize, int16_t{0});
  return written;
}

audio::Clock::duration AVDecoder::getDuration() const
{
  return toDuration<audio::Clock::duration>(videoStream->stream->duration, videoStream->stream->time_base);
}

int AVDecoder::getSampleRate() const
{
  return audioStream->context->sample_rate;
}
} // namespace video
