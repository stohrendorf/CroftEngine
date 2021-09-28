#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texture2d.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

// IWYU pragma: no_forward_declare gl::Framebuffer
// IWYU pragma: no_forward_declare gl::Texture2D
// IWYU pragma: no_forward_declare gl::TextureDepth
// IWYU pragma: no_forward_declare gl::TextureHandle

namespace render::pass
{
class GeometryPass
{
public:
  explicit GeometryPass(const glm::ivec2& viewport);
  ~GeometryPass();
  void bind(const glm::ivec2& size);

  [[nodiscard]] auto getNormalBuffer() const
  {
    return gsl::not_null{m_normalBufferHandle};
  }

  [[nodiscard]] auto getPositionBuffer() const
  {
    return gsl::not_null{m_positionBufferHandle};
  }

  [[nodiscard]] auto getInterpolatedPositionBuffer() const
  {
    return gsl::not_null{m_interpolatedPositionBufferHandle};
  }

  [[nodiscard]] auto getColorBuffer() const
  {
    return gsl::not_null{m_colorBufferHandle};
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

private:
  gsl::not_null<std::shared_ptr<gl::TextureDepth<float>>> m_depthBuffer;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_colorBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::RGB32F>>> m_positionBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_positionBufferHandle;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_interpolatedPositionBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::RGB16F>>> m_normalBuffer;
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>> m_normalBufferHandle;
  std::shared_ptr<gl::Framebuffer> m_fb;
};
} // namespace render::pass
