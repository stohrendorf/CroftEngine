#pragma once

#include <gl/pixel.h>

namespace loader::file
{
struct Palette;
struct ByteColor;
} // namespace loader::file

namespace render::scene
{
class Material;
}

namespace ui
{
struct BoxGouraud
{
  gl::SRGBA8 topLeft;
  gl::SRGBA8 topRight;
  gl::SRGBA8 bottomRight;
  gl::SRGBA8 bottomLeft;
};

extern void drawOutlineBox(const std::shared_ptr<render::scene::Material>& material,
                           const glm::ivec2& xy,
                           const glm::ivec2& size,
                           const loader::file::Palette& palette);
extern void drawBox(const std::shared_ptr<render::scene::Material>& material,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const BoxGouraud& gouraud);
extern void drawBox(const std::shared_ptr<render::scene::Material>& material,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const gl::SRGBA8& color);
} // namespace ui
