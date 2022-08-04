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

namespace render::scene
{
class Camera;
class Material;
class Mesh;
class MaterialManager;

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
    std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::RG16F>>> squaredTextureHandle;
    std::shared_ptr<gl::Framebuffer> squareFramebuffer{};
    std::shared_ptr<Material> squareMaterial{};
    std::shared_ptr<Mesh> squareMesh{};
    std::shared_ptr<SeparableBlur<gl::RG16F>> squareBlur;
    mutable std::unique_ptr<gl::FenceSync> depthSync;
    mutable std::unique_ptr<gl::FenceSync> squareSync;
    mutable std::unique_ptr<gl::FenceSync> blurSync;

    void init(int32_t resolution, size_t idx, MaterialManager& materialManager);
    void renderSquare();
    void renderBlur();
  };

  explicit CSM(int32_t resolution, MaterialManager& materialManager);

  [[nodiscard]] std::array<gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::RG16F>>>, CSMBuffer::NSplits>
    getTextures() const;
  [[nodiscard]] std::array<gslu::nn_shared<gl::TextureDepth<float>>, CSMBuffer::NSplits> getDepthTextures() const;
  [[nodiscard]] std::array<glm::mat4, CSMBuffer::NSplits> getMatrices(const glm::mat4& modelMatrix) const;

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix) const
  {
    return m_splits.at(m_activeSplit).vpMatrix * modelMatrix;
  }

  [[nodiscard]] const auto& getActiveFramebuffer() const
  {
    return m_splits.at(m_activeSplit).depthFramebuffer;
  }

  void beginActiveDepthSync() const
  {
    m_splits.at(m_activeSplit).depthSync = std::make_unique<gl::FenceSync>();
  }

  void waitActiveDepthSync() const
  {
    auto& sync = m_splits.at(m_activeSplit).depthSync;
    if(sync == nullptr)
      return;

    sync->wait();
  }

  void beginActiveSquareSync() const
  {
    m_splits.at(m_activeSplit).squareSync = std::make_unique<gl::FenceSync>();
  }

  void waitActiveSquareSync() const
  {
    auto& sync = m_splits.at(m_activeSplit).squareSync;
    if(sync == nullptr)
      return;

    sync->wait();
  }

  void beginActiveBlurSync() const
  {
    m_splits.at(m_activeSplit).blurSync = std::make_unique<gl::FenceSync>();
  }

  void waitActiveBlurSync() const
  {
    auto& sync = m_splits.at(m_activeSplit).blurSync;
    if(sync == nullptr)
      return;

    sync->wait();
  }

  void setActiveSplit(size_t idx)
  {
    Expects(idx < m_splits.size());
    m_activeSplit = idx;
  }

  void updateCamera(const Camera& camera);

  gl::UniformBuffer<CSMBuffer>& getBuffer(const glm::mat4& modelMatrix);

  void renderSquare();

  void renderBlur()
  {
    m_splits.at(m_activeSplit).renderBlur();
  }

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
