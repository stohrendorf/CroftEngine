#include "avdecoder.h"

#include "audiostreamdecoder.h"
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
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
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
  audioDecoder = std::make_unique<AudioStreamDecoder>(fmtContext, true);

#ifndef NDEBUG
  av_dump_format(fmtContext, 0, filename.c_str(), 0);
#endif

  filterGraph.init(*videoStream);

  Expects(videoStream->stream->time_base.den != 0);
  audioFrameSize
    = audioDecoder->getSampleRate() * videoStream->stream->time_base.num / videoStream->stream->time_base.den;

  Expects(av_new_packet(&packet, 0) == 0);
  fillQueues();
}

AVDecoder::~AVDecoder()
{
  avformat_close_input(&fmtContext);
}

void AVDecoder::fillQueues()
{
  {
    std::unique_lock lock(imgQueueMutex);
    if(audioDecoder->filled() || imgQueue.size() >= QueueLimit)
    {
      return;
    }
  }

  int err;
  while((err = av_read_frame(fmtContext, &packet)) == 0)
  {
    if(packet.stream_index == videoStream->index)
    {
      decodeVideoPacket();
    }
    else if(packet.stream_index == audioDecoder->stream->index)
    {
      audioDecoder->push(packet);
    }

    av_packet_unref(&packet);

    std::unique_lock lock(imgQueueMutex);
    if(audioDecoder->filled() || imgQueue.size() >= QueueLimit)
    {
      break;
    }
  }
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if(err != 0 && err != AVERROR_EOF)
  {
    BOOST_LOG_TRIVIAL(warning) << "fillQueues done: " << getAvError(err);
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

  stopped = imgQueue.empty() && audioDecoder->empty();

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

size_t AVDecoder::read(int16_t* buffer, size_t bufferSize, bool /*looping*/)
{
  fillQueues();

  size_t written = 0;
  {
    written = audioDecoder->read(buffer, bufferSize);
    std::unique_lock lock{imgQueueMutex};
    if(written == 0 && !imgQueue.empty())
    {
      // audio ended prematurely - pad with zero audio data until all video frames are consumed
      written = bufferSize;
    }
  }

  bufferSize -= written;
  buffer += audioDecoder->getChannels() * written;
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

  std::fill_n(buffer, audioDecoder->getChannels() * bufferSize, int16_t{0});
  return written;
}

audio::Clock::duration AVDecoder::getDuration() const
{
  return toDuration<audio::Clock::duration>(videoStream->stream->duration, videoStream->stream->time_base);
}

int AVDecoder::getSampleRate() const
{
  return audioDecoder->getSampleRate();
}

int AVDecoder::getChannels() const
{
  return audioDecoder->getChannels();
}
} // namespace video
