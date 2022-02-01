#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texture2d.h> // IWYU pragma: keep
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
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
                      const gslu::nn_shared<gl::TextureDepth<float>>& depthBuffer,
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
  gslu::nn_shared<gl::Texture2D<gl::Scalar32F>> m_positionBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>> m_positionBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::RGB32F>> m_perturbBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_perturbBufferHandle;
  scene::SeparableBlur<gl::RGB32F> m_blur;
  gslu::nn_shared<gl::Framebuffer> m_fb;
};
} // namespace render::pass
