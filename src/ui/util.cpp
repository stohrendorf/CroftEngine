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
  for(int y = 0; y < size.y; ++y)
  {
    const auto left = gl::mix(gouraud.topLeft, gouraud.bottomLeft, static_cast<float>(y) / static_cast<float>(size.y));
    const auto right
      = gl::mix(gouraud.topRight, gouraud.bottomRight, static_cast<float>(y) / static_cast<float>(size.y));

    for(int x = 0; x < size.x; ++x)
    {
      const auto c = gl::mix(left, right, static_cast<float>(x) / static_cast<float>(size.x));
      img.set(xy + glm::ivec2{x, y}, c, blend);
    }
  }
}
} // namespace ui
