#include "gl/image.h"

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace ui
{
extern void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                           const glm::ivec2& xy,
                           const glm::ivec2& size,
                           const loader::file::Palette& palette);

extern void drawLine(gl::Image<gl::SRGBA8>& img,
                     const glm::ivec2& xy0,
                     const glm::ivec2& size,
                     const loader::file::ByteColor& color);
} // namespace ui
