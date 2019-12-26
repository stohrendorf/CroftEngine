#pragma once

#include "core/vec.h"
#include "render/gl/buffer.h"
#include "render/gl/framebuffer.h"
#include "render/gl/texture.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace render::scene
{
class Camera;

struct CSMBuffer
{
  static constexpr size_t NSplits = 3;

  alignas(16) std::array<glm::mat4, NSplits> lightMVP{};
  alignas(16) std::array<glm::vec4, NSplits> csmSplits{}; // because... well. we need padding.
};

class CSM final
{
public:
  struct Split final
  {
    glm::mat4 pvMatrix{1.0f};
    const std::shared_ptr<gl::TextureDepth> texture{std::make_shared<gl::TextureDepth>()};
    std::shared_ptr<gl::Framebuffer> framebuffer{};
    float end = 0;

    void init(int32_t resolution, size_t idx);
  };

  explicit CSM(int32_t resolution);

  [[nodiscard]] auto getSplits() const
  {
    return m_splits.size();
  }

  [[nodiscard]] const auto& getSplit(size_t idx) const
  {
    return m_splits.at(idx);
  }

  void applyViewport();

  [[nodiscard]] std::array<std::shared_ptr<gl::TextureDepth>, CSMBuffer::NSplits> getTextures() const;

  [[nodiscard]] const auto& getTexture(size_t idx) const
  {
    return m_splits.at(idx).texture;
  }

  [[nodiscard]] std::array<glm::mat4, CSMBuffer::NSplits> getMatrices(const glm::mat4& modelMatrix) const;

  [[nodiscard]] std::array<float, CSMBuffer::NSplits> getSplitEnds() const;

  [[nodiscard]] const auto& getActiveTexture() const
  {
    return m_splits.at(m_activeSplit).texture;
  }

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix) const
  {
    return m_splits.at(m_activeSplit).pvMatrix * modelMatrix;
  }

  [[nodiscard]] const auto& getActiveFramebuffer() const
  {
    return m_splits.at(m_activeSplit).framebuffer;
  }

  void update(const Camera& camera);

  void setActiveSplit(size_t idx)
  {
    Expects(idx < m_splits.size());
    m_activeSplit = idx;
  }

  auto& getBuffer(const glm::mat4& modelMatrix)
  {
    const auto tmp = getSplitEnds();
    std::transform(tmp.begin(), tmp.end(), m_bufferData.csmSplits.begin(), [](float x) { return glm::vec4{x}; });
    m_bufferData.lightMVP = getMatrices(modelMatrix);
    m_buffer.setData(m_bufferData, ::gl::BufferUsageARB::DynamicDraw);
    return m_buffer;
  }

private:
  const int32_t m_resolution;
  const glm::vec3 m_lightDir{core::TRVec{0_len, 1_len, 0_len}.toRenderSystem()};
  const glm::vec3 m_lightDirOrtho{core::TRVec{1_len, 0_len, 0_len}.toRenderSystem()};
  std::array<Split, CSMBuffer::NSplits> m_splits;
  size_t m_activeSplit = 0;
  CSMBuffer m_bufferData;
  gl::ShaderStorageBuffer<CSMBuffer> m_buffer{};
};
} // namespace render::scene
