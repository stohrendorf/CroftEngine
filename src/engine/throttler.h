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
    m_delayRatios.fill(0.0f);
  }

  void wait()
  {
    // frame rate throttling
    // TODO this assumes that the frame rate capacity (the processing power so to speak)
    //      is faster than 30 FPS.
    const auto delay
      = std::chrono::duration_cast<TimeType>(m_nextFrameTime - std::chrono::high_resolution_clock::now()).count();
    m_delayRatios[m_delayRatioIdx] = static_cast<float>(delay) / static_cast<float>(FrameDuration.count());
    m_delayRatioIdx = (m_delayRatioIdx + 1u) % AverageSamples;

    std::this_thread::sleep_until(m_nextFrameTime);
    m_nextFrameTime += FrameDuration;
  }

  void reset()
  {
    m_nextFrameTime = std::chrono::high_resolution_clock::now() + FrameDuration;
  }

  // the higher the value, the better. should never exceed 1 (best performance). can be negative if the machine is too slow.
  [[nodiscard]] auto getAverageDelayRatio() const
  {
    return std::accumulate(m_delayRatios.begin(), m_delayRatios.end(), 0.0f, std::plus<>())
           / gsl::narrow_cast<float>(m_delayRatios.size());
  }

private:
  using TimeType = std::chrono::microseconds;
  static constexpr TimeType FrameDuration
    = std::chrono::duration_cast<TimeType>(std::chrono::seconds(1)) / core::FrameRate.get();
  static constexpr size_t AverageSamples = 30;

  std::chrono::high_resolution_clock::time_point m_nextFrameTime{};
  size_t m_delayRatioIdx{0};
  std::array<float, AverageSamples> m_delayRatios{};
};
} // namespace engine
