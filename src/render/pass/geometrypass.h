#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texturedepth.h>
#include <glm/fwd.hpp>

namespace render::pass
{
class GeometryPass
{
public:
  explicit GeometryPass(const glm::ivec2& viewport);
  void bind(const glm::ivec2& size);

  [[nodiscard]] const auto& getNormalBuffer() const
  {
    return m_normalBuffer;
  }

  [[nodiscard]] const auto& getPositionBuffer() const
  {
    return m_positionBuffer;
  }

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBuffer;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

private:
  std::shared_ptr<gl::TextureDepth<float>> m_depthBuffer;
  std::shared_ptr<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  std::shared_ptr<gl::Texture2D<gl::RGB32F>> m_positionBuffer;
  std::shared_ptr<gl::Texture2D<gl::RGB16F>> m_normalBuffer;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
