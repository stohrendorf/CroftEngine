#pragma once

#include <gl/image.h>

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace ui
{
struct alignas(16) BoxGouraud
{
  gl::SRGBA8 topLeft;
  gl::SRGBA8 topRight;
  gl::SRGBA8 bottomRight;
  gl::SRGBA8 bottomLeft;
};

extern void drawOutlineBox(gl::Image<gl::SRGBA8>& img,
                           const glm::ivec2& xy,
                           const glm::ivec2& size,
                           const loader::file::Palette& palette);
extern void drawBox(gl::Image<gl::SRGBA8>& img,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const BoxGouraud& gouraud,
                    bool blend = false);
} // namespace ui
