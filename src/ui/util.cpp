#include "util.h"

#include "loader/file/color.h"

namespace ui
{
void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const loader::file::Palette& palette)
{
  const auto color1 = palette.colors[15].toTextureColor();
  const auto color2 = palette.colors[31].toTextureColor();

  // top
  img.lineDxy(xy - glm::ivec2{0, 1}, {size.x + 1, 0}, color1);
  img.lineDxy(xy, {size.x, 0}, color2);
  //right
  img.lineDxy(xy + glm::ivec2{size.x, -1}, {0, size.y + 1}, color1);
  img.lineDxy(xy + glm::ivec2{size.x + 1, -1}, {0, size.y + 2}, color2);
  // bottom
  img.lineDxy(xy + size, {-size.x, 0}, color1);
  img.lineDxy(xy + size + glm::ivec2{0, 1}, {-size.x - 1, 0}, color2);
  // left
  img.lineDxy(xy + glm::ivec2{-1, size.y}, {0, -size.y - 1}, color1);
  img.lineDxy(xy + glm::ivec2{0, size.y}, {0, -size.y}, color2);
}

void drawBox(
  gl::Image<gl::SRGBA8>& img, const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud, bool blend)
{
  Expects(size.x >= 0 && size.y >= 0);

  for(uint32_t y = 0; y < static_cast<uint32_t>(size.y); ++y)
  {
    const auto left = gl::imix(gouraud.topLeft, gouraud.bottomLeft, y, static_cast<uint32_t>(size.y));
    const auto right = gl::imix(gouraud.topRight, gouraud.bottomRight, y, static_cast<uint32_t>(size.y));

    for(uint32_t x = 0; x < static_cast<uint32_t>(size.x); ++x)
    {
      const auto c = gl::imix(left, right, x, static_cast<uint32_t>(size.x));
      img.set(xy + glm::ivec2{x, y}, c, blend);
    }
  }
}
} // namespace ui
