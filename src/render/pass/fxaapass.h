#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gl::Framebuffer
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureHandle

namespace render::scene
{
class MaterialManager;
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
    return m_colorBufferHandle;
  }

private:
  const gsl::not_null<std::shared_ptr<scene::Material>> m_material;
  gsl::not_null<std::shared_ptr<scene::Mesh>> m_mesh;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_colorBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
