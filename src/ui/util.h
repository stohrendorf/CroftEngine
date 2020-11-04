#include "gl/image.h"

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace ui
{
extern void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                           const int x,
                           const int y,
                           const int width,
                           const int height,
                           const loader::file::Palette& palette);

extern void drawLine(gl::Image<gl::SRGBA8>& img,
                     const int x0,
                     const int y0,
                     const int width,
                     const int height,
                     const loader::file::ByteColor& color);
} // namespace ui
