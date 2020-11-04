#include "util.h"

#include "loader/file/color.h"

namespace ui
{
void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                    const int x,
                    const int y,
                    const int width,
                    const int height,
                    const loader::file::Palette& palette)
{
  // top
  drawLine(img, x, y - 1, width + 1, 0, palette.colors[15]);
  drawLine(img, x, y, width, 0, palette.colors[31]);
  //right
  drawLine(img, x + width, y - 1, 0, height + 1, palette.colors[15]);
  drawLine(img, x + width + 1, y - 1, 0, height + 2, palette.colors[31]);
  // bottom
  drawLine(img, x + width, y + height, -width, 0, palette.colors[15]);
  drawLine(img, x + width, y + height + 1, -width - 1, 0, palette.colors[31]);
  // left
  drawLine(img, x - 1, y + height, 0, -height - 1, palette.colors[15]);
  drawLine(img, x, y + height, 0, -height, palette.colors[31]);
}

void drawLine(gl::Image<gl::SRGBA8>& img,
              const int x0,
              const int y0,
              const int width,
              const int height,
              const loader::file::ByteColor& color)
{
  img.line(x0, y0, x0 + width, y0 + height, color.toTextureColor());
}
} // namespace ui
