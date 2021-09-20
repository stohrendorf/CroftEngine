#include "ffmpegstreamsource.h"

#include "audiostreamdecoder.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
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

BasicFfmpegStreamSource::BasicFfmpegStreamSource(AVFormatContext* fmtContext, const gsl::czstring& streamName)
    : m_fmtContext{fmtContext}
{
  if(const auto err = avformat_open_input(&m_fmtContext, streamName, nullptr, nullptr); err < 0)
  {
    BOOST_LOG_TRIVIAL(fatal) << getAvError(err);
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not open source file"));
  }

  if(const auto err = avformat_find_stream_info(m_fmtContext, nullptr); err < 0)
  {
    BOOST_LOG_TRIVIAL(fatal) << getAvError(err);
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream information"));
  }

  m_decoder = std::make_unique<AudioStreamDecoder>(m_fmtContext, false);

#ifndef NDEBUG
  av_dump_format(m_fmtContext, 0, streamName, 0);
#endif

  Expects(av_new_packet(&m_packet, 0) == 0);
  fillQueues(false);
}

BasicFfmpegStreamSource::~BasicFfmpegStreamSource()
{
  avformat_close_input(&m_fmtContext);
}

void BasicFfmpegStreamSource::fillQueues(bool looping)
{
  while(!m_decoder->filled())
  {
    const auto err = av_read_frame(m_fmtContext, &m_packet);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(err == AVERROR_EOF && looping)
    {
      m_decoder->seek(std::chrono::milliseconds{0});
      continue;
    }
    else if(err != 0)
    {
      break;
    }

    m_decoder->push(m_packet);
    av_packet_unref(&m_packet);
  }
}

size_t BasicFfmpegStreamSource::readStereo(int16_t* buffer, size_t bufferSize, bool looping)
{
  fillQueues(looping);
  size_t written = 0;
  while(written < bufferSize)
  {
    const auto read = m_decoder->readStereo(buffer, bufferSize - written);
    written += read;
    buffer += 2 * read;
    if(written < bufferSize)
    {
      if(!looping)
        break;
      m_decoder->seek(std::chrono::milliseconds{0});
    }
  }
  return written;
}

audio::Clock::duration BasicFfmpegStreamSource::getDuration() const
{
  return m_decoder->getDuration();
}

int BasicFfmpegStreamSource::getSampleRate() const
{
  return m_decoder->getSampleRate();
}

std::chrono::milliseconds BasicFfmpegStreamSource::getPosition() const
{
  return m_decoder->getPosition();
}

void BasicFfmpegStreamSource::seek(const std::chrono::milliseconds& position)
{
  m_decoder->seek(position);
  fillQueues(false);
}

FfmpegStreamSource::FfmpegStreamSource(const std::filesystem::path& filename)
    : BasicFfmpegStreamSource{nullptr, filename.string().c_str()}
{
}

FfmpegMemoryStreamSource::FfmpegMemoryStreamSource(const gsl::span<const uint8_t>& data)
    : detail::FfmpegMemoryStreamSourceFileData{data}
    , BasicFfmpegStreamSource{
        [this]()
        {
          auto ctx = avformat_alloc_context();
          ctx->pb = avio_alloc_context(
            dataBuffer, 0, 0, this, &FfmpegMemoryStreamSource::read, nullptr, &FfmpegMemoryStreamSource::seek);
          return ctx;
        }(),
        nullptr}
{
}

FfmpegMemoryStreamSource::~FfmpegMemoryStreamSource()
{
  av_free(getFmtContext()->pb->buffer);
  avio_context_free(&getFmtContext()->pb);
}

int FfmpegMemoryStreamSource::read(void* opaque, uint8_t* buf, int bufSize)
{
  auto* h = static_cast<FfmpegMemoryStreamSource*>(opaque);
  const auto n = std::min(h->data.size() - h->dataPosition, gsl::narrow<size_t>(bufSize));
  if(n == 0)
    return AVERROR_EOF;

  std::copy_n(&h->data[h->dataPosition], n, buf);
  h->dataPosition += n;
  return n;
}

int64_t FfmpegMemoryStreamSource::seek(void* opaque, int64_t offset, int whence)
{
  auto* h = static_cast<FfmpegMemoryStreamSource*>(opaque);
  if((whence & AVSEEK_SIZE) != 0)
    return h->data.size();

  switch(whence)
  {
  case SEEK_SET:
    h->dataPosition = gsl::narrow_cast<size_t>(std::clamp(offset, int64_t{0}, gsl::narrow<int64_t>(h->data.size())));
    break;
  case SEEK_CUR:
    h->dataPosition = gsl::narrow_cast<size_t>(gsl::narrow_cast<size_t>(
      std::clamp(gsl::narrow<int64_t>(h->dataPosition + offset), int64_t{0}, gsl::narrow<int64_t>(h->data.size()))));
    break;
  case SEEK_END:
    h->dataPosition = gsl::narrow_cast<size_t>(
      std::clamp(gsl::narrow<int64_t>(h->data.size() - offset), int64_t{0}, gsl::narrow<int64_t>(h->data.size())));
    break;
  default: BOOST_THROW_EXCEPTION(std::domain_error("invalid whence value"));
  }
  return gsl::narrow_cast<int64_t>(h->dataPosition);
}

FfmpegSubStreamStreamSource::FfmpegSubStreamStreamSource(std::unique_ptr<std::istream>&& istream,
                                                         size_t start,
                                                         size_t end)
    : detail::FfmpegMemoryStreamSourceSubFileData{std::move(istream), start, end}
    , BasicFfmpegStreamSource{
        [this]()
        {
          auto ctx = avformat_alloc_context();
          ctx->pb = avio_alloc_context(
            dataBuffer, 0, 0, this, &FfmpegSubStreamStreamSource::read, nullptr, &FfmpegSubStreamStreamSource::seek);
          return ctx;
        }(),
        nullptr}
{
}

FfmpegSubStreamStreamSource::~FfmpegSubStreamStreamSource()
{
  av_free(getFmtContext()->pb->buffer);
  avio_context_free(&getFmtContext()->pb);
}

int FfmpegSubStreamStreamSource::read(void* opaque, uint8_t* buf, int bufSize)
{
  auto* h = static_cast<FfmpegSubStreamStreamSource*>(opaque);
  const auto n = std::min(h->dataEnd - h->dataStart - h->dataPosition, gsl::narrow<size_t>(bufSize));
  h->istream->seekg(h->dataStart + h->dataPosition);
  h->istream->read(reinterpret_cast<char*>(buf), n);
  h->dataPosition += n;
  return n;
}

int64_t FfmpegSubStreamStreamSource::seek(void* opaque, int64_t offset, int whence)
{
  auto* h = static_cast<FfmpegSubStreamStreamSource*>(opaque);
  const auto size = h->dataEnd - h->dataStart;
  if((whence & AVSEEK_SIZE) != 0)
    return size;

  switch(whence)
  {
  case SEEK_SET:
    h->dataPosition = gsl::narrow_cast<size_t>(std::clamp(offset, int64_t{0}, gsl::narrow<int64_t>(size)));
    break;
  case SEEK_CUR:
    h->dataPosition = gsl::narrow_cast<size_t>(gsl::narrow_cast<size_t>(
      std::clamp(gsl::narrow<int64_t>(h->dataPosition + offset), int64_t{0}, gsl::narrow<int64_t>(size))));
    break;
  case SEEK_END:
    h->dataPosition = gsl::narrow_cast<size_t>(
      std::clamp(gsl::narrow<int64_t>(size - offset), int64_t{0}, gsl::narrow<int64_t>(size)));
    break;
  default: BOOST_THROW_EXCEPTION(std::domain_error("invalid whence value"));
  }
  return gsl::narrow_cast<int64_t>(h->dataPosition);
}
} // namespace video
