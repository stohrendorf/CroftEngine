#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <mutex>
#include <tuple>
#include <vector>

class Bitmap
{
public:
  explicit Bitmap(const glm::ivec2& size);
  explicit Bitmap(const glm::ivec2& size, const gsl::span<const uint32_t>& rgba);
  virtual ~Bitmap() = default;

  [[nodiscard]] uint32_t* data() noexcept
  {
    return m_data.data();
  }
  [[nodiscard]] const uint32_t* data() const noexcept
  {
    return m_data.data();
  }
  [[nodiscard]] const glm::ivec2& size() const noexcept
  {
    return m_size;
  }

  [[nodiscard]] std::tuple<const uint32_t*, uint32_t, bool> nextBlock(uint32_t wantedLines);

protected:
  explicit Bitmap(uint32_t lines);

  std::vector<uint32_t> m_data;
  uint32_t* m_block = nullptr;
  uint32_t m_linesLeft;
  glm::ivec2 m_size{};
  std::mutex m_lock;
};
