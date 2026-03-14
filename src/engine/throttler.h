#pragma once

#include "core/magic.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace engine
{
class Throttler
{
public:
  Throttler()
      : m_now{std::chrono::high_resolution_clock::now()}
      , m_nextLogicTick{std::chrono::high_resolution_clock::now()}
  {
    reset();
  }

  [[nodiscard]] bool isTimeForLogic() const noexcept
  {
    return m_isLogicTick;
  }

  void tick()
  {
    m_now = std::chrono::high_resolution_clock::now();
    m_isLogicTick = m_now >= m_nextLogicTick;
    if(m_isLogicTick)
    {
      m_nextLogicTick += LogicTickDuration;
    }

    const auto elapsed = m_now - (m_nextLogicTick - LogicTickDuration);
    const auto factor = std::chrono::duration_cast<std::chrono::duration<float>>(elapsed).count()
                        / std::chrono::duration_cast<std::chrono::duration<float>>(LogicTickDuration).count();
    m_interTickFactor = std::clamp(factor, 0.0f, 1.0f);
  }

  [[nodiscard]] float getInterTickFactor() const noexcept
  {
    return m_interTickFactor;
  }

  void reset() noexcept
  {
    m_now = std::chrono::high_resolution_clock::now();
    m_nextLogicTick = m_now + LogicTickDuration;
    m_interTickFactor = 0;
    m_isLogicTick = true;
  }

private:
  using TimeType = std::chrono::microseconds;
  static constexpr TimeType LogicTickDuration = std::chrono::duration_cast<TimeType>(core::TimePerFrame);

  std::chrono::high_resolution_clock::time_point m_now;
  std::chrono::high_resolution_clock::time_point m_nextLogicTick;
  float m_interTickFactor = 0;
  bool m_isLogicTick = true;
};
} // namespace engine
