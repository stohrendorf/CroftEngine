#pragma once

#include <chrono>
#include <optional>

namespace engine
{
class Stopwatch
{
public:
  explicit Stopwatch() = default;

  [[nodiscard]] bool running() const
  {
    return m_start.has_value();
  }

  void start()
  {
    m_start = std::chrono::high_resolution_clock::now();
  }

  [[nodiscard]] std::chrono::milliseconds stop()
  {
    if(!m_start.has_value())
    {
      return std::chrono::milliseconds{0};
    }

    auto tmp = std::chrono::high_resolution_clock::now() - *m_start;
    m_start.reset();
    return std::chrono::duration_cast<std::chrono::milliseconds>(tmp);
  }

private:
  std::optional<std::chrono::high_resolution_clock::time_point> m_start{};
};
} // namespace engine
