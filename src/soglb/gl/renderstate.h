#pragma once

#include "api/gl.hpp"

#include <boost/assert.hpp>
#include <glm/glm.hpp>
#include <optional>

namespace gl
{
class RenderState final
{
public:
  RenderState(const RenderState&) noexcept = default;
  RenderState(RenderState&&) noexcept = default;
  RenderState& operator=(const RenderState&) noexcept = default;
  RenderState& operator=(RenderState&&) noexcept = default;
  explicit RenderState() noexcept = default;
  ~RenderState() noexcept = default;

  void setBlend(const bool enabled)
  {
    m_blendEnabled = enabled;
  }

  void setBlendSrc(const api::BlendingFactor blend)
  {
    m_blendSrc = blend;
  }

  void setBlendDst(const api::BlendingFactor blend)
  {
    m_blendDst = blend;
  }

  void setCullFace(const bool enabled)
  {
    m_cullFaceEnabled = enabled;
  }

  void setCullFaceSide(const api::CullFaceMode side)
  {
    m_cullFaceSide = side;
  }

  void setFrontFace(const api::FrontFaceDirection winding)
  {
    m_frontFace = winding;
  }

  void setDepthTest(const bool enabled)
  {
    m_depthTestEnabled = enabled;
  }

  void setDepthWrite(const bool enabled)
  {
    m_depthWriteEnabled = enabled;
  }

  void setDepthClamp(const bool enabled)
  {
    m_depthClampEnabled = enabled;
  }

  void setDepthFunction(const api::DepthFunction func)
  {
    m_depthFunction = func;
  }

  void setLineWidth(const float width)
  {
    m_lineWidth = width;
  }

  void setLineSmooth(const bool enabled)
  {
    m_lineSmooth = enabled;
  }

  void setViewport(const glm::ivec2& viewport)
  {
    m_viewport = viewport;
  }

  void setProgram(const uint32_t program)
  {
    m_program = program;
  }

  static RenderState getDefaults();

  static RenderState& getWantedState();
  static void applyWantedState();
  static void resetWantedState()
  {
    getWantedState() = getDefaults();
  }

  void merge(const RenderState& other);

private:
  void apply(bool force = false) const;

  // States
  std::optional<glm::ivec2> m_viewport{};
  std::optional<uint32_t> m_program{};
  std::optional<bool> m_cullFaceEnabled{};
  std::optional<bool> m_depthTestEnabled{};
  std::optional<bool> m_depthWriteEnabled{};
  std::optional<bool> m_depthClampEnabled{};
  std::optional<api::DepthFunction> m_depthFunction{};
  std::optional<bool> m_blendEnabled{};
  std::optional<api::BlendingFactor> m_blendSrc{};
  std::optional<api::BlendingFactor> m_blendDst{};
  std::optional<api::CullFaceMode> m_cullFaceSide{};
  std::optional<api::FrontFaceDirection> m_frontFace{};
  std::optional<float> m_lineWidth{};
  std::optional<bool> m_lineSmooth{};
};
} // namespace gl
