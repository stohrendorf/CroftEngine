#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <sndfile.h> // IWYU pragma: export

namespace audio::sndfile
{
class MemBufferFileIo : public SF_VIRTUAL_IO
{
  friend sf_count_t memBufferGetFileLength(void* user_data);
  friend sf_count_t memBufferDoSeek(sf_count_t offset, int whence, void* user_data);
  friend sf_count_t memBufferDoRead(void* ptr, sf_count_t count, void* user_data);
  friend sf_count_t memBufferDoWrite(const void* ptr, sf_count_t count, void* user_data);
  friend sf_count_t memBufferDoTell(void* user_data);

public:
  MemBufferFileIo(const uint8_t* data, sf_count_t dataSize);

private:
  const uint8_t* const m_data;
  const sf_count_t m_dataSize;
  sf_count_t m_where = 0;
};

class InputStreamViewWrapper : public SF_VIRTUAL_IO
{
  friend sf_count_t streamGetFileLength(void* user_data);
  friend sf_count_t streamDoSeek(sf_count_t offset, int whence, void* user_data);
  friend sf_count_t streamDoRead(void* ptr, sf_count_t count, void* user_data);
  friend sf_count_t streamDoWrite(const void* ptr, sf_count_t count, void* user_data);
  friend sf_count_t streamDoTell(void* user_data);

public:
  InputStreamViewWrapper(std::istream& stream, std::streamoff begin, std::streamoff end);
  ~InputStreamViewWrapper();

private:
  struct BoostStreamImpl;
  std::unique_ptr<BoostStreamImpl> m_streamImpl;
};
} // namespace audio::sndfile
