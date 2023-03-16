#pragma once

#include <gl/fencesync.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gl/texturedepth.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::pass
{
class GeometryPass
{
public:
  explicit GeometryPass(const glm::ivec2& viewport);
  ~GeometryPass();
  void render(const std::function<void()>& doRender);

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

  [[nodiscard]] const auto& getDepthBufferHandle() const
  {
    return m_depthBufferHandle;
  }

  [[nodiscard]] const auto& getReflectiveBuffer() const
  {
    return m_reflectiveBufferHandle;
  }

  void wait()
  {
    if(m_sync == nullptr)
      return;

    m_sync->wait();
    m_sync.reset();
  }

private:
  gslu::nn_shared<gl::TextureDepth<float>> m_depthBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::TextureDepth<float>>> m_depthBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_colorBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_colorBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::SRGBA8>> m_reflectiveBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> m_reflectiveBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::RGB32F>> m_positionBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RGB32F>>> m_positionBufferHandle;
  gslu::nn_shared<gl::Texture2D<gl::RGB16F>> m_normalBuffer;
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RGB16F>>> m_normalBufferHandle;
  gslu::nn_shared<gl::Framebuffer> m_fb;
  std::unique_ptr<gl::FenceSync> m_sync;
};
} // namespace render::pass
