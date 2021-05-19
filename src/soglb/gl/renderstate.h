#pragma once

#include "api/gl.hpp"

#include <boost/assert.hpp>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace gl
{
class Texture;

class RenderState final
{
public:
  RenderState(const RenderState&) noexcept = default;
  RenderState(RenderState&&) noexcept = default;
  RenderState& operator=(const RenderState&) = default;
  RenderState& operator=(RenderState&&) = default;

  explicit RenderState() noexcept = default;
  ~RenderState() noexcept = default;

  void setBlend(const bool enabled)
  {
    m_blendEnabled = enabled;
  }

  void setBlendFactors(const api::BlendingFactor src, const api::BlendingFactor dst)
  {
    setBlendFactors(src, src, dst, dst);
  }

  void setBlendFactors(const api::BlendingFactor srcRgb,
                       const api::BlendingFactor srcAlpha,
                       const api::BlendingFactor dstRgb,
                       const api::BlendingFactor dstAlpha)
  {
    m_blendFactors = {srcRgb, srcAlpha, dstRgb, dstAlpha};
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
  std::optional<std::tuple<api::BlendingFactor, api::BlendingFactor, api::BlendingFactor, api::BlendingFactor>>
    m_blendFactors{};
  std::optional<api::CullFaceMode> m_cullFaceSide{};
  std::optional<api::FrontFaceDirection> m_frontFace{};
  std::optional<float> m_lineWidth{};
  std::optional<bool> m_lineSmooth{};
};
} // namespace gl
