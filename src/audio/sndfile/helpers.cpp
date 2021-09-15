#include "helpers.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/restrict.hpp>
#include <cstdio>
#include <gsl/gsl-lite.hpp>
#include <istream>
#include <string>

namespace audio::sndfile
{
struct InputStreamViewWrapper::BoostStreamImpl
{
  std::unique_ptr<boost::iostreams::restriction<std::istream>> restriction;
  std::unique_ptr<boost::iostreams::filtering_istream> stream;

  BoostStreamImpl(std::istream& stream, const std::streamoff begin, const std::streamoff end)
      : restriction{std::make_unique<boost::iostreams::restriction<std::istream>>(
        boost::iostreams::restrict(stream, begin, end - begin))}
      , stream{std::make_unique<boost::iostreams::filtering_istream>(*restriction)}
  {
  }
};

sf_count_t memBufferGetFileLength(void* user_data)
{
  const auto self = static_cast<MemBufferFileIo*>(user_data);
  return self->m_dataSize;
}

sf_count_t memBufferDoSeek(const sf_count_t offset, const int whence, void* user_data)
{
  auto self = static_cast<MemBufferFileIo*>(user_data);
  switch(whence)
  {
  case SEEK_SET:
    BOOST_ASSERT(offset >= 0 && offset <= self->m_dataSize);
    self->m_where = offset;
    break;
  case SEEK_CUR:
    if(offset < 0 && -offset > self->m_dataSize)
      self->m_where = 0;
    else
      self->m_where = std::min(self->m_where + offset, self->m_dataSize);
    break;
  case SEEK_END:
    if(offset > 0 && offset > self->m_dataSize)
      self->m_where = 0;
    else
      self->m_where = std::min(self->m_dataSize - offset, self->m_dataSize);
    break;
  default: BOOST_ASSERT(false);
  }
  return self->m_where;
}

sf_count_t memBufferDoRead(void* ptr, sf_count_t count, void* user_data)
{
  auto self = static_cast<MemBufferFileIo*>(user_data);
  if(self->m_where + count > self->m_dataSize)
    count = self->m_dataSize - self->m_where;

  BOOST_ASSERT(self->m_where + count <= self->m_dataSize);

  const auto buf = static_cast<uint8_t*>(ptr);
  std::copy_n(self->m_data + self->m_where, count, buf);
  self->m_where += count;
  return count;
}

sf_count_t memBufferDoWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
{
  return 0; // read-only
}

sf_count_t memBufferDoTell(void* user_data)
{
  const auto self = static_cast<MemBufferFileIo*>(user_data);
  return self->m_where;
}

sf_count_t streamGetFileLength(void* user_data)
{
  auto self = static_cast<InputStreamViewWrapper*>(user_data);
  const auto pos = self->m_streamImpl->stream->tellg();
  self->m_streamImpl->stream->seekg(0, std::ios::end);
  const auto len = self->m_streamImpl->stream->tellg();
  self->m_streamImpl->stream->seekg(pos, std::ios::beg);
  return len;
}

sf_count_t streamDoSeek(const sf_count_t offset, const int whence, void* user_data)
{
  auto self = static_cast<InputStreamViewWrapper*>(user_data);
  switch(whence)
  {
  case SEEK_SET:
    BOOST_ASSERT(offset >= 0 && offset <= streamGetFileLength(user_data));
    self->m_streamImpl->stream->seekg(offset, std::ios::beg);
    break;
  case SEEK_CUR:
    BOOST_ASSERT(self->m_streamImpl->stream->tellg() + offset <= streamGetFileLength(user_data)
                 && self->m_streamImpl->stream->tellg() + offset >= 0);
    self->m_streamImpl->stream->seekg(offset, std::ios::cur);
    break;
  case SEEK_END:
    BOOST_ASSERT(offset >= 0 && offset <= streamGetFileLength(user_data));
    self->m_streamImpl->stream->seekg(offset, std::ios::end);
    break;
  default: BOOST_ASSERT(false);
  }
  return self->m_streamImpl->stream->tellg();
}

sf_count_t streamDoRead(void* ptr, sf_count_t count, void* user_data)
{
  auto self = static_cast<InputStreamViewWrapper*>(user_data);
  if(self->m_streamImpl->stream->tellg() + count > streamGetFileLength(user_data))
    count = streamGetFileLength(user_data) - self->m_streamImpl->stream->tellg();

  BOOST_ASSERT(self->m_streamImpl->stream->tellg() + count <= streamGetFileLength(user_data));

  const auto buf = static_cast<char*>(ptr);
  self->m_streamImpl->stream->read(buf, count);
  return self->m_streamImpl->stream->gcount();
}

sf_count_t streamDoWrite(const void* /*ptr*/, sf_count_t /*count*/, void* /*user_data*/)
{
  return 0; // read-only
}

sf_count_t streamDoTell(void* user_data)
{
  auto self = static_cast<InputStreamViewWrapper*>(user_data);
  return self->m_streamImpl->stream->tellg();
}

MemBufferFileIo::MemBufferFileIo(const uint8_t* data, const sf_count_t dataSize)
    : SF_VIRTUAL_IO{}
    , m_data{data}
    , m_dataSize{dataSize}
{
  BOOST_ASSERT(data != nullptr);

  get_filelen = &memBufferGetFileLength;
  seek = &memBufferDoSeek;
  read = &memBufferDoRead;
  write = &memBufferDoWrite;
  tell = &memBufferDoTell;
}

InputStreamViewWrapper::InputStreamViewWrapper(std::istream& stream,
                                               const std::streamoff begin,
                                               const std::streamoff end)
    : SF_VIRTUAL_IO{}
    , m_streamImpl{std::make_unique<BoostStreamImpl>(stream, begin, end)}
{
  Expects(begin <= end);

  get_filelen = &streamGetFileLength;
  seek = &streamDoSeek;
  read = &streamDoRead;
  write = &streamDoWrite;
  tell = &streamDoTell;
}

InputStreamViewWrapper::~InputStreamViewWrapper() = default;
} // namespace audio::sndfile
