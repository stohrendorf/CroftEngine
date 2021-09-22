#pragma once

#include "core/magic.h"

#include <chrono>
#include <numeric>
#include <thread>

namespace engine
{
class Throttler
{
public:
  Throttler()
      : m_nextFrameTime{std::chrono::high_resolution_clock::now() + FrameDuration}
  {
    m_waitRatios.fill(0.0f);
  }

  void wait()
  {
    // frame rate throttling
    const auto now = std::chrono::high_resolution_clock::now();
    const auto wait = std::chrono::duration_cast<TimeType>(m_nextFrameTime - now).count();
    m_waitRatios[m_waitRatioIdx] = static_cast<float>(wait) / static_cast<float>(FrameDuration.count());
    m_waitRatioIdx = (m_waitRatioIdx + 1u) % AverageSamples;

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

  void reset()
  {
    m_nextFrameTime = std::chrono::high_resolution_clock::now() + FrameDuration;
  }

  // the higher the value, the better. should never exceed 1 (best performance). can be negative if the machine is too slow.
  [[nodiscard]] auto getAverageWaitRatio() const
  {
    return std::accumulate(m_waitRatios.begin(), m_waitRatios.end(), 0.0f, std::plus<>())
           / gsl::narrow_cast<float>(m_waitRatios.size());
  }

private:
  using TimeType = std::chrono::microseconds;
  static constexpr TimeType FrameDuration
    = std::chrono::duration_cast<TimeType>(std::chrono::seconds(1)) / core::FrameRate.get();
  static constexpr size_t AverageSamples = 30;

  std::chrono::high_resolution_clock::time_point m_nextFrameTime{};
  size_t m_waitRatioIdx{0};
  std::array<float, AverageSamples> m_waitRatios{};
};
} // namespace engine
