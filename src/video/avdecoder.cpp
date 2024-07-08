#include "avdecoder.h"

#include "audio/audiostreamdecoder.h"
#include "audio/core.h"
#include "ffmpeg/avframeptr.h"
#include "ffmpeg/stream.h"
#include "ffmpeg/util.h"
#include "filtergraph.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
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

  videoStream = std::make_unique<ffmpeg::Stream>(fmtContext, AVMEDIA_TYPE_VIDEO, false);
  audioDecoder = std::make_unique<audio::AudioStreamDecoder>(fmtContext, true);

#ifndef NDEBUG
  av_dump_format(fmtContext, 0, filename.c_str(), 0);
#endif

  filterGraph.init(*videoStream);

  gsl_Assert(av_new_packet(&packet, 0) == 0);
  fillQueues();
  stopped = imgQueue.empty() && audioDecoder->empty();
}

AVDecoder::~AVDecoder()
{
  avformat_close_input(&fmtContext);
}

void AVDecoder::fillQueues()
{
  {
    const std::unique_lock lock(imgQueueMutex);
    if(audioDecoder->filled())
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

    const std::unique_lock lock(imgQueueMutex);
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

std::optional<ffmpeg::AVFramePtr> AVDecoder::takeFrame()
{
  std::unique_lock lock{imgQueueMutex};
  if(stopped)
  {
    return std::nullopt;
  }

  std::optional<ffmpeg::AVFramePtr> img;
  if(!imgQueue.empty())
  {
    const auto delta = getVideoTs() - std::chrono::high_resolution_clock::now();

    if(delta.count() > 0)
    {
      // video frame is in the future
      lock.unlock();
      std::this_thread::sleep_for(delta);
      lock.lock();
    }

    img = std::move(imgQueue.front());
    imgQueue.pop();
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

  ffmpeg::AVFramePtr videoFrame;
  int err;
  while((err = avcodec_receive_frame(videoStream->context, videoFrame.frame)) == 0)
  {
    if(const auto addFrameErr = av_buffersrc_add_frame(filterGraph.input, videoFrame.release()))
    {
      BOOST_LOG_TRIVIAL(error) << "Error while feeding the filtergraph: " << getAvError(addFrameErr);
      BOOST_THROW_EXCEPTION(std::runtime_error("Error while feeding the filtergraph"));
    }
    videoFrame = ffmpeg::AVFramePtr();

    while(true)
    {
      ffmpeg::AVFramePtr filteredFrame;
      const auto ret = av_buffersink_get_frame(filterGraph.output, filteredFrame.frame);
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;
      if(ret < 0)
      {
        BOOST_LOG_TRIVIAL(error) << "Filter error: " << getAvError(ret);
        BOOST_THROW_EXCEPTION(std::runtime_error("Filter error"));
      }

      const std::unique_lock lock(imgQueueMutex);
      imgQueue.push(std::move(filteredFrame));
    }
  }
  if(err != AVERROR(EAGAIN))
    BOOST_LOG_TRIVIAL(info) << "Video stream chunk decoded: " << getAvError(err);
}

size_t AVDecoder::read(int16_t* buffer, size_t bufferSize, bool /*looping*/)
{
  fillQueues();

  size_t written;
  {
    written = audioDecoder->read(buffer, bufferSize);
    const std::unique_lock lock{imgQueueMutex};
    if(written == 0 && !imgQueue.empty())
    {
      // audio ended prematurely - pad with zero audio data until all video frames are consumed
      written = bufferSize;
    }
  }

  if(written == 0)
  {
    std::fill_n(buffer, audioDecoder->getChannels() * bufferSize, int16_t{0});
    return written;
  }

  bufferSize -= written;
  buffer += audioDecoder->getChannels() * written;
  totalAudioFrames += written;
  std::fill_n(buffer, audioDecoder->getChannels() * bufferSize, int16_t{0});
  return written;
}

audio::Clock::duration AVDecoder::getDuration() const
{
  return ffmpeg::toDuration<audio::Clock::duration>(videoStream->stream->duration, videoStream->stream->time_base);
}

int AVDecoder::getSampleRate() const
{
  return audioDecoder->getSampleRate();
}

int AVDecoder::getChannels() const
{
  return audioDecoder->getChannels();
}

std::chrono::high_resolution_clock::time_point AVDecoder::getVideoTs()
{
  gsl_Assert(!imgQueue.empty());

  return m_playStart
         + ffmpeg::toDuration<std::chrono::high_resolution_clock::duration>(imgQueue.front().frame->pts,
                                                                            videoStream->stream->time_base);
}
} // namespace video
