#pragma once

#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texture2d.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

namespace render::pass
{
class GeometryPass
{
public:
  explicit GeometryPass(const glm::ivec2& viewport);
  ~GeometryPass();
  void bind();

  [[nodiscard]] const auto& getNormalBuffer() const
  {
    return m_normalBufferHandle;
  }

  [[nodiscard]] const auto& getPositionBuffer() const
  {
    return m_positionBufferHandle;
  }

  [[nodiscard]] const auto& getColorBuffer() const
  {
    return m_colorBufferHandle;
  }

  [[nodiscard]] const auto& getDepthBuffer() const
  {
    return m_depthBuffer;
  }

private:
  gsl::not_null<std::shared_ptr<gl::TextureDepth<float>>> m_depthBuffer;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::SRGBA8>>> m_colorBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>> m_colorBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::RGB32F>>> m_positionBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>>> m_positionBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Texture2D<gl::RGB16F>>> m_normalBuffer;
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>>> m_normalBufferHandle;
  gsl::not_null<std::shared_ptr<gl::Framebuffer>> m_fb;
};
} // namespace render::pass
