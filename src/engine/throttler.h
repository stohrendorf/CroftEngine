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
    // TODO this assumes that the frame rate capacity (the processing power so to speak)
    // is faster than 30 FPS.
    std::this_thread::sleep_until(m_nextFrameTime);
    m_nextFrameTime += FrameDuration;
  }

  void reset()
  {
    m_nextFrameTime = std::chrono::high_resolution_clock::now() + FrameDuration;
  }

private:
  static constexpr std::chrono::microseconds FrameDuration
    = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1)) / core::FrameRate.get();

  std::chrono::high_resolution_clock::time_point m_nextFrameTime{};
};
} // namespace engine
