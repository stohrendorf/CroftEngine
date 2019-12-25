#pragma once

#include "core/vec.h"
#include "render/gl/framebuffer.h"
#include "render/gl/texture.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace render::scene
{
class Camera;

class CSM final
{
public:
  struct Split final
  {
    glm::mat4 pvMatrix{1.0f};
    const std::shared_ptr<gl::TextureDepth> texture{std::make_shared<gl::TextureDepth>()};
    std::shared_ptr<gl::Framebuffer> framebuffer{};
    float end = 0;

    explicit Split(int32_t resolution);
  };

  explicit CSM(uint8_t splits, int32_t resolution);

  [[nodiscard]] auto getSplits() const
  {
    return m_splits.size();
  }

  [[nodiscard]] const auto& getSplit(size_t idx) const
  {
    return m_splits.at(idx);
  }

  void applyViewport();

  [[nodiscard]] std::vector<std::shared_ptr<gl::TextureDepth>> getTextures() const;

  [[nodiscard]] const auto& getTexture(size_t idx) const
  {
    return m_splits.at(idx).texture;
  }

  [[nodiscard]] std::vector<glm::mat4> getMatrices(const glm::mat4& modelMatrix = glm::mat4{1.0f}) const;

  [[nodiscard]] std::vector<float> getSplitEnds() const;

  [[nodiscard]] const auto& getActiveTexture() const
  {
    return m_splits.at(m_activeSplit).texture;
  }

  [[nodiscard]] auto getActiveMatrix(const glm::mat4& modelMatrix = glm::mat4{1.0f}) const
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

private:
  const int32_t m_resolution;
  const glm::vec3 m_lightDir{core::TRVec{0_len, 1_len, 0_len}.toRenderSystem()};
  const glm::vec3 m_lightDirOrtho{core::TRVec{1_len, 0_len, 0_len}.toRenderSystem()};
  std::vector<Split> m_splits;
  size_t m_activeSplit = 0;
};
} // namespace render::scene
