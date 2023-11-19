#pragma once

#include "api/gl.hpp"

#include <array>
#include <cstdint>
#include <glm/vec2.hpp>
#include <optional>
#include <tuple>

namespace gl
{
class RenderState final
{
public:
  static constexpr uint32_t IndexedCaps = 8;

  RenderState(const RenderState&) noexcept = default;
  RenderState(RenderState&&) noexcept = default;
  RenderState& operator=(const RenderState&) = default;
  RenderState& operator=(RenderState&&) = default;

  explicit RenderState() noexcept = default;
  ~RenderState() noexcept = default;

  void setBlend(const uint32_t index, const bool enabled)
  {
    m_blendEnabled.at(index) = enabled;
  }

  void setBlendFactors(const uint32_t index, const api::BlendingFactor src, const api::BlendingFactor dst)
  {
    setBlendFactors(index, src, src, dst, dst);
  }

  void setBlendFactors(const uint32_t index,
                       const api::BlendingFactor srcRgb,
                       const api::BlendingFactor srcAlpha,
                       const api::BlendingFactor dstRgb,
                       const api::BlendingFactor dstAlpha)
  {
    m_blendFactors.at(index) = {srcRgb, srcAlpha, dstRgb, dstAlpha};
  }

  void setCullFace(const bool enabled)
  {
    m_cullFaceEnabled = enabled;
  }

  void setCullFaceSide(const api::TriangleFace face)
  {
    m_cullFaceSide = face;
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

  void setScissorTest(bool enabled)
  {
    m_scissorTest = enabled;
  }

  [[nodiscard]] const auto& getScissorTest() const
  {
    return m_scissorTest;
  }

  void setScissorRegion(const glm::vec2& xy, const glm::vec2& size)
  {
    m_scissorRegion = {xy, size};
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

  void setPolygonOffsetFill(bool enabled)
  {
    m_polygonOffsetFillEnabled = enabled;
  }

  void setPolygonOffset(float factor, float units)
  {
    m_polygonOffset = {factor, units};
  }

  static void reset();

private:
  void apply() const;

  // States
  std::optional<glm::ivec2> m_viewport{};
  std::optional<uint32_t> m_program{};
  std::optional<bool> m_cullFaceEnabled{};
  std::optional<bool> m_depthTestEnabled{};
  std::optional<bool> m_depthWriteEnabled{};
  std::optional<bool> m_depthClampEnabled{};
  std::optional<api::DepthFunction> m_depthFunction{};
  std::array<std::optional<bool>, IndexedCaps> m_blendEnabled{};
  std::array<
    std::optional<std::tuple<api::BlendingFactor, api::BlendingFactor, api::BlendingFactor, api::BlendingFactor>>,
    IndexedCaps>
    m_blendFactors{};
  std::optional<api::TriangleFace> m_cullFaceSide{};
  std::optional<api::FrontFaceDirection> m_frontFace{};
  std::optional<float> m_lineWidth{};
  std::optional<bool> m_lineSmooth{};
  std::optional<bool> m_scissorTest{};
  std::optional<std::tuple<glm::vec2, glm::vec2>> m_scissorRegion{};
  std::optional<bool> m_polygonOffsetFillEnabled{};
  std::optional<std::tuple<float, float>> m_polygonOffset{};
};
} // namespace gl
