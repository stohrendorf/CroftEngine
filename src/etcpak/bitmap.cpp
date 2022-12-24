#include "bitmap.hpp"

Bitmap::Bitmap(const glm::ivec2& size)
    : m_linesLeft{static_cast<uint32_t>(size.y / 4)}
    , m_size{size}
{
  m_data.resize(size.x * size.y);
  m_block = m_data.data();
}

Bitmap::Bitmap(const glm::ivec2& size, const gsl::span<const uint32_t>& rgba)
    : m_linesLeft{static_cast<uint32_t>(size.y / 4)}
    , m_size{size}
{
  gsl_Assert(size.x * size.y == rgba.size());

  m_data.resize(size.x * size.y);
  std::copy(rgba.begin(), rgba.end(), m_data.begin());
  // convert to the expected BGRA input from RGBA
  for(auto& px : m_data)
  {
    auto ptr = reinterpret_cast<uint8_t*>(&px);
    std::swap(ptr[0], ptr[2]);
  }
  m_block = m_data.data();
}

Bitmap::Bitmap(uint32_t lines)
    : m_linesLeft{lines}
{
}

std::tuple<const uint32_t*, uint32_t, bool> Bitmap::nextBlock(uint32_t wantedLines)
{
  const std::lock_guard<std::mutex> lock(m_lock);
  const auto lines = std::min(wantedLines, m_linesLeft);
  auto ret = m_block;
  m_block += m_size.x * 4 * lines;
  m_linesLeft -= lines;
  return {ret, lines, m_linesLeft == 0};
}
