#include "ffmpegstreamsource.h"

#include "audiostreamdecoder.h"

#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>
#include <string>

extern "C"
{
#include <libavformat/avformat.h>
}

namespace video
{
FfmpegStreamSource::FfmpegStreamSource(const std::filesystem::path& filename)
{
  if(avformat_open_input(&fmtContext, filename.string().c_str(), nullptr, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not open source file"));
  }

  if(avformat_find_stream_info(fmtContext, nullptr) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream information"));
  }

  decoder = std::make_unique<AudioStreamDecoder>(fmtContext, false);

#ifndef NDEBUG
  av_dump_format(fmtContext, 0, filename.string().c_str(), 0);
#endif

  Expects(av_new_packet(&packet, 0) == 0);
  fillQueues(false);
}

FfmpegStreamSource::~FfmpegStreamSource()
{
  avformat_close_input(&fmtContext);
}

void FfmpegStreamSource::fillQueues(bool looping)
{
  while(!decoder->filled())
  {
    const auto err = av_read_frame(fmtContext, &packet);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(err == AVERROR_EOF && looping)
    {
      decoder->seek(std::chrono::milliseconds{0});
      continue;
    }
    else if(err != 0)
    {
      break;
    }

    decoder->push(packet);
    av_packet_unref(&packet);
  }
}

size_t FfmpegStreamSource::readStereo(int16_t* buffer, size_t bufferSize, bool looping)
{
  fillQueues(looping);
  size_t written = 0;
  while(written < bufferSize)
  {
    const auto read = decoder->readStereo(buffer, bufferSize - written);
    written += read;
    buffer += 2 * read;
    if(written < bufferSize)
    {
      if(!looping)
        break;
      decoder->seek(std::chrono::milliseconds{0});
    }
  }
  return written;
}

audio::Clock::duration FfmpegStreamSource::getDuration() const
{
  return decoder->getDuration();
}

int FfmpegStreamSource::getSampleRate() const
{
  return decoder->getSampleRate();
}

std::chrono::milliseconds FfmpegStreamSource::getPosition() const
{
  return decoder->getPosition();
}

void FfmpegStreamSource::seek(const std::chrono::milliseconds& position)
{
  decoder->seek(position);
  fillQueues(false);
}
} // namespace video
