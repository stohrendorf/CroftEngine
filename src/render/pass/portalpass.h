#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texture2d.h> // IWYU pragma: keep
#include <glm/fwd.hpp>
#include <memory>

namespace render::scene
{
class MaterialManager;
}

namespace render::pass
{
class PortalPass
{
public:
  explicit PortalPass(scene::MaterialManager& materialManager,
                      const gsl::not_null<std::shared_ptr<gl::TextureDepth<float>>>& depthBuffer,
                      const glm::vec2& viewport);

  [[nodiscard]] gl::RenderState bind();

  void renderBlur()
  {
    m_blur.render();
  }

  [[nodiscard]] auto getPositionBuffer() const
  {
    return gsl::not_null{m_positionBufferHandle};
  }

  [[nodiscard]] auto getNoisyTexture() const
  {
    return gsl::not_null{m_perturbBufferHandle};
  }

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

private:
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::Scalar32F>>> m_positionBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>>> m_positionBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::RGB32F>>> m_perturbBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>> m_perturbBufferHandle;
  scene::SeparableBlur<gl::RGB32F> m_blur;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
