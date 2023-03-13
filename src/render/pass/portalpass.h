#pragma once

#include "render/scene/blur.h"

#include <functional>
#include <gl/fencesync.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texture2d.h> // IWYU pragma: keep
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::material
{
class MaterialManager;
}

namespace render::pass
{
class PortalPass
{
public:
  explicit PortalPass(material::MaterialManager& materialManager,
                      const gslu::nn_shared<gl::TextureDepth<float>>& depthBuffer,
                      const glm::vec2& viewport);

  void render(const std::function<void(const gl::RenderState&)>& doRender);

  void renderBlur()
  {
    m_blur.render();
  }

  [[nodiscard]] const auto& getPositionBuffer() const
  {
    return m_positionBufferHandle;
  }

  [[nodiscard]] const auto& getNoisyTexture() const
  {
    return m_perturbBufferHandle;
  }

  [[nodiscard]] auto getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

  void wait()
  {
    if(m_sync != nullptr)
    {
      m_sync->wait();
      m_sync.reset();
    }
  }

private:
  gslu::nn_shared<gl::Texture2D<gl::Scalar32F>> m_positionBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::Scalar32F>>> m_positionBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::RGB32F>> m_perturbBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_perturbBufferHandle;
  scene::SeparableBlur<gl::RGB32F> m_blur;
  gslu::nn_shared<gl::Framebuffer> m_fb;
  std::unique_ptr<gl::FenceSync> m_sync;
};
} // namespace render::pass
