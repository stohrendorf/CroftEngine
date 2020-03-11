#pragma once

#include "core/vec.h"
#include "material.h"

#include <cstdint>
#include <gl/buffer.h>
#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/texturedepth.h>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace render::scene
{
class Camera;

struct alignas(16) CSMBuffer
{
  static constexpr size_t NSplits = 3;

  std::array<glm::mat4, NSplits> lightMVP{};
  std::array<glm::vec4, NSplits> csmSplits{}; // vec4 because... well. we need padding.
  glm::vec3 lightDir;
};

class CSM final
{
public:
  struct Split final
  {
    glm::mat4 vpMatrix{1.0f};
    std::shared_ptr<gl::TextureDepth<int32_t>> texture;
    std::shared_ptr<gl::Framebuffer> framebuffer{};
    float end = 0;

    void init(int32_t resolution, size_t idx);
  };

  explicit CSM(int32_t resolution);

  [[nodiscard]] std::array<std::shared_ptr<gl::TextureDepth<int32_t>>, CSMBuffer::NSplits> getTextures() const;
  [[nodiscard]] std::array<glm::mat4, CSMBuffer::NSplits> getMatrices(const glm::mat4& modelMatrix) const;
  [[nodiscard]] std::array<float, CSMBuffer::NSplits> getSplitEnds() const;

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix) const
  {
    return m_splits.at(m_activeSplit).vpMatrix * modelMatrix;
  }

  [[nodiscard]] const auto& getActiveFramebuffer() const
  {
    return m_splits.at(m_activeSplit).framebuffer;
  }

  void setActiveSplit(size_t idx)
  {
    Expects(idx < m_splits.size());
    m_activeSplit = idx;
  }

  void update(const Camera& camera);

  auto& getBuffer(const glm::mat4& modelMatrix)
  {
    const auto tmp = getSplitEnds();
    std::transform(tmp.begin(), tmp.end(), m_bufferData.csmSplits.begin(), [](float x) { return glm::vec4{x}; });
    m_bufferData.lightMVP = getMatrices(modelMatrix);
    m_bufferData.lightDir = m_lightDir;
    m_buffer.setData(m_bufferData, gl::api::BufferUsageARB::DynamicDraw);
    return m_buffer;
  }

  void applyViewport()
  {
    GL_ASSERT(gl::api::viewport(0, 0, m_resolution, m_resolution));
  }

private:
  const int32_t m_resolution;
  const glm::vec3 m_lightDir{core::TRVec{0_len, 1_len, 0_len}.toRenderSystem()};
  const glm::vec3 m_lightDirOrtho{core::TRVec{1_len, 0_len, 0_len}.toRenderSystem()};
  std::array<Split, CSMBuffer::NSplits> m_splits;
  size_t m_activeSplit = 0;
  CSMBuffer m_bufferData;
  gl::UniformBuffer<CSMBuffer> m_buffer{"csm-data-ubo"};
};
} // namespace render::scene
