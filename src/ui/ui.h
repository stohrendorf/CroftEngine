#pragma once

#include <array>
#include <gl/pixel.h>
#include <gsl-lite.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace render::scene
{
class Mesh;
class Material;
} // namespace render::scene

namespace ui
{
struct BoxGouraud;

class Ui final
{
public:
  explicit Ui(std::shared_ptr<render::scene::Material> material, const std::array<gl::SRGBA8, 256>& palette)
      : m_material{std::move(material)}
      , m_palette{palette}
  {
  }

  void drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color);

  void render();

private:
  const std::shared_ptr<render::scene::Material> m_material;
  const std::array<gl::SRGBA8, 256> m_palette;
  std::vector<gsl::not_null<std::shared_ptr<render::scene::Mesh>>> m_meshes{};
};
} // namespace ui
