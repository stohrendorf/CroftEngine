#pragma once

#include "core.h"
#include "handle.h"

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace audio
{
class BufferHandle : public Handle
{
public:
  explicit BufferHandle()
      : Handle{alGenBuffers, alIsBuffer, alDeleteBuffers}
  {
  }

  void fill(const int16_t* samples, size_t frameCount, int sampleRate);
  void fillFromWav(const uint8_t* data);

  [[nodiscard]] Clock::duration getDuration() const
  {
    return Clock::duration((m_sampleRate * Clock::duration::period::den)
                           / (m_frameCount * Clock::duration::period::num));
  }

private:
  size_t m_frameCount = 0;
  int m_sampleRate = 0;
};
} // namespace audio
