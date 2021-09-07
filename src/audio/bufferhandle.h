#pragma once

#include <AL/al.h>
#include <cstdint>

namespace audio
{
class BufferHandle final
{
public:
  explicit BufferHandle();

  explicit BufferHandle(const BufferHandle&) = delete;
  explicit BufferHandle(BufferHandle&&) = delete;
  BufferHandle& operator=(const BufferHandle&) = delete;
  BufferHandle& operator=(BufferHandle&&) = delete;

  ~BufferHandle();

  [[nodiscard]] ALuint get() const noexcept
  {
    return m_handle;
  }

  void fill(const int16_t* samples, size_t sampleCount, int channels, int sampleRate);

  bool fillFromWav(const uint8_t* data);

private:
  const ALuint m_handle{};
};
} // namespace audio
