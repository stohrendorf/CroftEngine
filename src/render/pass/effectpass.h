#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>

namespace render::scene
{
class Material;
class Mesh;
} // namespace render::scene

namespace render::pass
{
class EffectPass final
{
public:
  explicit EffectPass(std::string name,
                      gsl::not_null<std::shared_ptr<scene::Material>> material,
                      const gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>>& input);

  void render(bool inWater);

  [[nodiscard]] const auto& getOutput() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getFramebuffer() const
  {
    return m_fb;
  }

private:
  const std::string m_name;
  const gsl::not_null<std::shared_ptr<scene::Material>> m_material;
  gsl::not_null<std::shared_ptr<scene::Mesh>> m_mesh;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGB8>>> m_colorBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
