#include "gl/image.h"

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace ui
{
extern void
  drawOutlineBox(gl::Image<gl::SRGBA8>& img, int x, int y, int width, int height, const loader::file::Palette& palette);

extern void
  drawLine(gl::Image<gl::SRGBA8>& img, int x0, int y0, int width, int height, const loader::file::ByteColor& color);
} // namespace ui
