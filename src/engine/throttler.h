#pragma once

#include "core/magic.h"

#include <chrono>
#include <thread>

namespace engine
{
class Throttler
{
public:
  Throttler()
      : m_nextFrameTime{std::chrono::high_resolution_clock::now() + FrameDuration}
  {
  }

  void wait()
  {
    // frame rate throttling
    const auto now = std::chrono::high_resolution_clock::now();
    const auto wait = std::chrono::duration_cast<TimeType>(m_nextFrameTime - now).count();

    if(wait > 0)
    {
      std::this_thread::sleep_until(m_nextFrameTime);
      m_nextFrameTime += FrameDuration;
    }
    else
    {
      m_nextFrameTime = now + FrameDuration;
    }
  }

  void reset() noexcept
  {
    m_nextFrameTime = std::chrono::high_resolution_clock::now() + FrameDuration;
  }

private:
  using TimeType = std::chrono::microseconds;
  static constexpr TimeType FrameDuration = std::chrono::duration_cast<TimeType>(core::TimePerFrame);

  std::chrono::high_resolution_clock::time_point m_nextFrameTime;
};
} // namespace engine
