#pragma once

#include "core/vec.h"
#include "material.h"
#include "render/gl/buffer.h"
#include "render/gl/framebuffer.h"
#include "render/gl/pixel.h"
#include "render/gl/texture.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace render::scene
{
class Camera;
class Model;
class ShaderManager;

struct CSMBuffer
{
  static constexpr size_t NSplits = 3;
  static constexpr int32_t BlurExtent = 2;

  alignas(16) std::array<glm::mat4, NSplits> lightMVP{};
  alignas(16) std::array<glm::vec4, NSplits> csmSplits{}; // because... well. we need padding.
};

class CSM final
{
public:
  struct Split final
  {
    glm::mat4 pvMatrix{1.0f};
    std::shared_ptr<gl::TextureDepth> texture;
    std::shared_ptr<gl::Framebuffer> framebuffer{};
    float end = 0;
    std::shared_ptr<gl::Texture2D<gl::Scalar32F>> blurBuffer;
    std::shared_ptr<gl::Framebuffer> blurFramebuffer;
    std::shared_ptr<scene::Material> blurMaterial;

    void init(int32_t resolution, size_t idx, const gsl::not_null<std::shared_ptr<ShaderProgram>>& blurShader);
  };

  explicit CSM(int32_t resolution, ShaderManager& shaderManager);

  [[nodiscard]] std::array<std::shared_ptr<gl::TextureDepth>, CSMBuffer::NSplits> getTextures() const;
  [[nodiscard]] std::array<std::shared_ptr<gl::Texture2D<gl::Scalar32F>>, CSMBuffer::NSplits> getBlurBuffers() const;
  [[nodiscard]] std::array<glm::mat4, CSMBuffer::NSplits> getMatrices(const glm::mat4& modelMatrix) const;
  [[nodiscard]] std::array<float, CSMBuffer::NSplits> getSplitEnds() const;

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix) const
  {
    return m_splits.at(m_activeSplit).pvMatrix * modelMatrix;
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
    m_buffer.setData(m_bufferData, ::gl::BufferUsageARB::DynamicDraw);
    return m_buffer;
  }

  void applyViewport()
  {
    GL_ASSERT(::gl::viewport(0, 0, m_resolution, m_resolution));
  }

  void renderBlur();

private:
  const int32_t m_resolution;
  const glm::vec3 m_lightDir{core::TRVec{0_len, 1_len, 0_len}.toRenderSystem()};
  const glm::vec3 m_lightDirOrtho{core::TRVec{1_len, 0_len, 0_len}.toRenderSystem()};
  std::array<Split, CSMBuffer::NSplits> m_splits;
  size_t m_activeSplit = 0;
  CSMBuffer m_bufferData;
  gl::UniformBuffer<CSMBuffer> m_buffer{};
  const std::shared_ptr<Model> m_fbModel;
};
} // namespace render::scene
