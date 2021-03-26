#pragma once

#include "render/scene/blur.h"

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>

namespace render::scene
{
class MaterialManager;
}

namespace render::pass
{
class PortalPass
{
public:
  explicit PortalPass(scene::MaterialManager& materialManager, const glm::vec2& viewport);

  void bind(const gl::TextureDepth<float>& depth);

  void renderBlur()
  {
    m_blur.render();
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

  [[nodiscard]] const auto& getNoisyTexture() const
  {
    return m_perturbBuffer;
  }

  [[nodiscard]] const auto& getBlurredTexture() const
  {
    return m_blur.getBlurredTexture();
  }

private:
  std::shared_ptr<gl::TextureDepth<float>> m_depthBuffer;
  std::shared_ptr<gl::Texture2D<gl::RG32F>> m_perturbBuffer;
  scene::SeparableBlur<gl::RG32F> m_blur;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
