#pragma once

#include "gl/image.h"

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace ui
{
// top left, top right, bottom right, bottom left
using BoxGouraud = std::array<gl::SRGBA8, 4>;

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
