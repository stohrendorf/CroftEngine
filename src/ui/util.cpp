#include "util.h"

#include "loader/file/color.h"

namespace ui
{
void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const loader::file::Palette& palette)
{
  // top
  drawLine(img, xy - glm::ivec2{0, 1}, {size.x + 1, 0}, palette.colors[15]);
  drawLine(img, xy, {size.x, 0}, palette.colors[31]);
  //right
  drawLine(img, xy + glm::ivec2{size.x, -1}, {0, size.y + 1}, palette.colors[15]);
  drawLine(img, xy + glm::ivec2{size.x + 1, -1}, {0, size.y + 2}, palette.colors[31]);
  // bottom
  drawLine(img, xy + size, {-size.x, 0}, palette.colors[15]);
  drawLine(img, xy + size + glm::ivec2{0, 1}, {-size.x - 1, 0}, palette.colors[31]);
  // left
  drawLine(img, xy + glm::ivec2{-1, size.y}, {0, -size.y - 1}, palette.colors[15]);
  drawLine(img, xy + glm::ivec2{0, size.y}, {0, -size.y}, palette.colors[31]);
}

void drawLine(gl::Image<gl::SRGBA8>& img,
              const glm::ivec2& xy0,
              const glm::ivec2& size,
              const loader::file::ByteColor& color)
{
  img.line(xy0, xy0 + size, color.toTextureColor());
}
} // namespace ui
