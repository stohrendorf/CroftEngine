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
class Mesh;
class Material;
class MaterialManager;
} // namespace render::scene

namespace render::pass
{
class UIPass
{
public:
  explicit UIPass(scene::MaterialManager& materialManager, const glm::ivec2& viewport);
  void bind();

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  void render(float alpha);

private:
  const std::shared_ptr<scene::Material> m_material;
  std::shared_ptr<scene::Mesh> m_mesh;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_colorBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
