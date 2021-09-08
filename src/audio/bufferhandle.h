#pragma once

#include "core.h"

#include <AL/al.h>
#include <chrono>
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
  void fillFromWav(const uint8_t* data);

  [[nodiscard]] Clock::duration getDuration() const
  {
    return Clock::duration((m_sampleRate * Clock::duration::period::den)
                           / (m_sampleCount * Clock::duration::period::num));
  }

private:
  const ALuint m_handle{};
  size_t m_sampleCount = 0;
  int m_sampleRate = 0;
};
} // namespace audio
