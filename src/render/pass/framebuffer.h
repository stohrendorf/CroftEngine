#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace render::scene
{
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class Framebuffer final
{
public:
  explicit Framebuffer(const std::string& name,
                       gsl::not_null<std::shared_ptr<scene::Material>> material,
                       const glm::ivec2& size);

  [[nodiscard]] const auto& getOutput() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

  void bind();

  void render();

private:
  const gsl::not_null<std::shared_ptr<scene::Material>> m_material;
  gsl::not_null<std::shared_ptr<scene::Mesh>> m_mesh;
  gsl::not_null<std::shared_ptr<gl::TextureDepth<float>>> m_depthBuffer;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_colorBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
