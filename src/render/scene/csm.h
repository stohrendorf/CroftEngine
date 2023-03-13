#pragma once

#include "core/units.h"
#include "core/vec.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/fencesync.h>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>

namespace render::material
{
class Material;
class MaterialManager;
} // namespace render::material

namespace render::scene
{
class Camera;
class Mesh;

template<typename PixelT>
class SeparableBlur;

struct CSMBuffer
{
  static constexpr size_t NSplits = 4;

  std::array<glm::mat4, NSplits> lightMVP{};
  glm::vec4 lightDir{};
};
static_assert(sizeof(CSMBuffer) % 16 == 0);

class CSM final
{
public:
  struct Split final
  {
    glm::mat4 vpMatrix{1.0f};
    std::shared_ptr<gl::TextureHandle<gl::TextureDepth<float>>> depthTextureHandle;
    std::shared_ptr<gl::Framebuffer> depthFramebuffer{};
    mutable std::unique_ptr<gl::FenceSync> depthSync;

    std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RG16F>>> squaredTextureHandle;
    std::shared_ptr<gl::Framebuffer> squareFramebuffer{};
    mutable std::unique_ptr<gl::FenceSync> squareSync;

    std::shared_ptr<material::Material> squareMaterial{};
    std::shared_ptr<Mesh> squareMesh{};
    std::shared_ptr<SeparableBlur<gl::RG16F>> squareBlur;
    mutable std::unique_ptr<gl::FenceSync> blurSync;

    void init(int32_t resolution, size_t idx, material::MaterialManager& materialManager);
    void renderSquare();
    void renderBlur();
  };

  explicit CSM(int32_t resolution, material::MaterialManager& materialManager);

  [[nodiscard]] std::array<gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>, CSMBuffer::NSplits>
    getTextures() const;
  [[nodiscard]] std::array<glm::mat4, CSMBuffer::NSplits> getMatrices(const glm::mat4& modelMatrix) const;

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix) const
  {
    return m_splits.at(m_activeSplit).vpMatrix * modelMatrix;
  }

  void renderToActiveDepthBuffer(const std::function<void(const gl::RenderState&, const glm::mat4&)>& doRender) const;
  void renderSquareBuffers();
  void renderBlurBuffers();
  void waitBlurBuffers()
  {
    for(auto& split : m_splits)
    {
      gsl_Assert(split.blurSync != nullptr);
      split.blurSync->wait();
      split.blurSync.reset();
    }
    GL_ASSERT(gl::api::memoryBarrier(gl::api::MemoryBarrierMask::FramebufferBarrierBit));
  }

  void setActiveSplit(size_t idx)
  {
    gsl_Expects(idx < m_splits.size());
    m_activeSplit = idx;
  }

  void updateCamera(const Camera& camera);

  gl::UniformBuffer<CSMBuffer>& getBuffer(const glm::mat4& modelMatrix);

  [[nodiscard]] auto getResolution() const
  {
    return m_resolution;
  }

private:
  const int32_t m_resolution;
  const glm::vec3 m_lightDir{core::TRVec{0_len, 1_len, 0_len}.toRenderSystem()};
  const glm::vec3 m_lightDirOrtho{core::TRVec{1_len, 0_len, 0_len}.toRenderSystem()};
  std::array<Split, CSMBuffer::NSplits> m_splits;
  size_t m_activeSplit = 0;
  CSMBuffer m_bufferData;
  gl::UniformBuffer<CSMBuffer> m_buffer;
};
} // namespace render::scene
