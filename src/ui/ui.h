#pragma once

#include <array>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace render::scene
{
class Mesh;
class Material;
} // namespace render::scene

namespace engine::world
{
struct Sprite;
}

namespace ui
{
struct BoxGouraud;

class Ui final
{
public:
  explicit Ui(std::shared_ptr<render::scene::Material> texture,
              std::shared_ptr<render::scene::Material> color,
              const std::array<gl::SRGBA8, 256>& palette)
      : m_texture{std::move(texture)}
      , m_color{std::move(color)}
      , m_palette{palette}
  {
  }

  void drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size, uint8_t alpha = 255);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const size_t color)
  {
    drawBox(xy, size, m_palette.at(color));
  }

  void drawHLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color);
  void drawVLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color);
  void draw(const engine::world::Sprite& sprite, const glm::ivec2& xy);

  void render(const glm::vec2& screenSize);

private:
  const std::shared_ptr<render::scene::Material> m_texture;
  const std::shared_ptr<render::scene::Material> m_color;
  const std::array<gl::SRGBA8, 256> m_palette;
  std::vector<gsl::not_null<std::shared_ptr<render::scene::Mesh>>> m_meshes{};
};
} // namespace ui
