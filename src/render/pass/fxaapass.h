#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>

namespace render::scene
{
class MaterialManager;
class ShaderProgram;
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class GeometryPass;

class FXAAPass
{
public:
  explicit FXAAPass(scene::MaterialManager& materialManager,
                    const glm::ivec2& viewport,
                    const GeometryPass& geometryPass);

  void bind();

  void render(const glm::ivec2& size);

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBuffer;
  }

private:
  const std::shared_ptr<scene::Material> m_material;
  std::shared_ptr<scene::Mesh> m_mesh;
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
