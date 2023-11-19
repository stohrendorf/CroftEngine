#include "renderstate.h"

#include "api/gl.hpp"
#include "glassert.h"

#include <glm/common.hpp>
#include <gsl/gsl-lite.hpp>
#include <utility>

namespace gl
{
// NOLINTNEXTLINE(misc-no-recursion)
inline RenderState& getCurrentState(bool reset = false)
{
  static RenderState currentState{};
  static bool initialized = false;

  if(!initialized || reset)
  {
    currentState = RenderState{};
    auto oldWanted = RenderState::getWantedState();
    RenderState::resetWantedState();
    initialized = true;
    RenderState::getWantedState() = oldWanted;
    RenderState::applyWantedState();
  }
  return currentState;
}

// NOLINTNEXTLINE(misc-no-recursion)
void RenderState::apply() const
{
  // Update any state if...
  //   - it is forced
  //   - or it is explicitly set and different than the current state
  // NOLINTNEXTLINE(bugprone-macro-parentheses)
#define RS_CHANGED(m) (m.has_value() && m != getCurrentState().m)
  bool updateScissorRegion = false;
  if(RS_CHANGED(m_viewport))
  {
    updateScissorRegion = true;
    gsl_Assert(m_viewport.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::viewport(0, 0, m_viewport->x, m_viewport->y));
    getCurrentState().m_viewport = m_viewport;
  }
  if(RS_CHANGED(m_program))
  {
    gsl_Assert(m_program.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::useProgram(m_program.value()));
    getCurrentState().m_program = m_program;
  }
  for(uint32_t i = 0; i < IndexedCaps; ++i)
  {
    if(RS_CHANGED(m_blendEnabled[i]))
    {
      gsl_Assert(m_blendEnabled[i].has_value());
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      if(m_blendEnabled[i].value())
        GL_ASSERT(api::enable(api::EnableCap::Blend, i));
      else
        GL_ASSERT(api::disable(api::EnableCap::Blend, i));
      getCurrentState().m_blendEnabled[i] = m_blendEnabled[i];
    }
  }
  for(uint32_t i = 0; i < IndexedCaps; ++i)
  {
    if(RS_CHANGED(m_blendFactors[i]))
    {
      gsl_Assert(m_blendFactors[i].has_value());
      // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
      const auto [srcRgb, srcAlpha, dstRgb, dstAlpha] = m_blendFactors[i].value();
      GL_ASSERT(api::blendFuncSeparate(i, srcRgb, dstRgb, srcAlpha, dstAlpha));
      getCurrentState().m_blendFactors[i] = m_blendFactors[i];
    }
  }
  if(RS_CHANGED(m_cullFaceEnabled))
  {
    gsl_Assert(m_cullFaceEnabled.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_cullFaceEnabled.value())
      GL_ASSERT(api::enable(api::EnableCap::CullFace));
    else
      GL_ASSERT(api::disable(api::EnableCap::CullFace));
    getCurrentState().m_cullFaceEnabled = m_cullFaceEnabled;
  }
  if(RS_CHANGED(m_cullFaceSide))
  {
    gsl_Assert(m_cullFaceSide.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::cullFace(m_cullFaceSide.value()));
    getCurrentState().m_cullFaceSide = m_cullFaceSide;
  }
  if(RS_CHANGED(m_frontFace))
  {
    gsl_Assert(m_frontFace.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::frontFace(m_frontFace.value()));
    getCurrentState().m_frontFace = m_frontFace;
  }
  if(RS_CHANGED(m_lineWidth))
  {
    gsl_Assert(m_lineWidth.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::lineWidth(m_lineWidth.value()));
    getCurrentState().m_lineWidth = m_lineWidth;
  }
  if(RS_CHANGED(m_lineSmooth))
  {
    gsl_Assert(m_lineSmooth.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_lineSmooth.value())
      GL_ASSERT(api::enable(api::EnableCap::LineSmooth));
    else
      GL_ASSERT(api::disable(api::EnableCap::LineSmooth));
    getCurrentState().m_lineSmooth = m_lineSmooth;
  }
  if(RS_CHANGED(m_depthTestEnabled))
  {
    gsl_Assert(m_depthTestEnabled.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_depthTestEnabled.value())
      GL_ASSERT(api::enable(api::EnableCap::DepthTest));
    else
      GL_ASSERT(api::disable(api::EnableCap::DepthTest));
    getCurrentState().m_depthTestEnabled = m_depthTestEnabled;
  }
  if(RS_CHANGED(m_depthWriteEnabled))
  {
    gsl_Assert(m_depthWriteEnabled.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::depthMask(m_depthWriteEnabled.value()));
    getCurrentState().m_depthWriteEnabled = m_depthWriteEnabled;
  }
  if(RS_CHANGED(m_depthClampEnabled))
  {
    gsl_Assert(m_depthClampEnabled.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_depthClampEnabled.value())
      GL_ASSERT(api::enable(api::EnableCap::DepthClamp));
    else
      GL_ASSERT(api::disable(api::EnableCap::DepthClamp));
    getCurrentState().m_depthClampEnabled = m_depthClampEnabled;
  }
  if(RS_CHANGED(m_depthFunction))
  {
    gsl_Assert(m_depthFunction.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    GL_ASSERT(api::depthFunc(m_depthFunction.value()));
    getCurrentState().m_depthFunction = m_depthFunction;
  }
  if(RS_CHANGED(m_scissorTest))
  {
    gsl_Assert(m_scissorTest.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_scissorTest.value())
      GL_ASSERT(api::enable(api::EnableCap::ScissorTest));
    else
      GL_ASSERT(api::disable(api::EnableCap::ScissorTest));
    getCurrentState().m_scissorTest = m_scissorTest;
  }
  if(RS_CHANGED(m_scissorRegion) && getCurrentState().m_viewport.has_value())
  {
    getCurrentState().m_scissorRegion = m_scissorRegion;
    updateScissorRegion = true;
  }
  if(updateScissorRegion && getCurrentState().m_scissorRegion.has_value())
  {
    const auto vp = glm::vec2{*getCurrentState().m_viewport};
    const auto& [xy, size] = *getCurrentState().m_scissorRegion;
    const auto screenXy = glm::floor(vp * (xy + glm::vec2{1, 1}) * 0.5f);
    const auto screenSize = glm::ceil(vp * size * 0.5f);
    GL_ASSERT(api::scissor(gsl::narrow_cast<int32_t>(screenXy.x),
                           gsl::narrow_cast<int32_t>(screenXy.y),
                           gsl::narrow_cast<api::core::SizeType>(screenSize.x),
                           gsl::narrow_cast<api::core::SizeType>(screenSize.y)));
  }
  if(RS_CHANGED(m_polygonOffsetFillEnabled) || RS_CHANGED(m_polygonOffset))
  {
    gsl_Assert(m_polygonOffsetFillEnabled.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    if(m_polygonOffsetFillEnabled.value())
    {
      gsl_Assert(m_polygonOffset.has_value());
      GL_ASSERT(api::enable(api::EnableCap::PolygonOffsetFill));
      auto [factor, units] = *m_polygonOffset;
      GL_ASSERT(api::polygonOffset(factor, units));
      getCurrentState().m_polygonOffset = m_polygonOffset;
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::PolygonOffsetFill));
      GL_ASSERT(api::polygonOffset(0, 0));
      getCurrentState().m_polygonOffset = {0.0f, 0.0f};
    }
    getCurrentState().m_polygonOffsetFillEnabled = m_polygonOffsetFillEnabled;
  }
#undef RS_CHANGED
}

void RenderState::merge(const RenderState& other)
{
#define MERGE_OPT(n)                                               \
  if(other.n.has_value()) /* NOLINT(bugprone-macro-parentheses) */ \
  n = other.n             /* NOLINT(bugprone-macro-parentheses) */
  MERGE_OPT(m_viewport);
  MERGE_OPT(m_cullFaceEnabled);
  MERGE_OPT(m_depthTestEnabled);
  MERGE_OPT(m_depthWriteEnabled);
  MERGE_OPT(m_depthClampEnabled);
  MERGE_OPT(m_depthFunction);
  for(uint32_t i = 0; i < IndexedCaps; ++i)
    MERGE_OPT(m_blendEnabled[i]);
  for(uint32_t i = 0; i < IndexedCaps; ++i)
    MERGE_OPT(m_blendFactors[i]);
  MERGE_OPT(m_cullFaceSide);
  MERGE_OPT(m_frontFace);
  MERGE_OPT(m_lineWidth);
  MERGE_OPT(m_lineSmooth);
  MERGE_OPT(m_scissorTest);
  MERGE_OPT(m_scissorRegion);
  MERGE_OPT(m_polygonOffsetFillEnabled);
  MERGE_OPT(m_polygonOffset);
#undef MERGE_OPT
}

RenderState& RenderState::getWantedState()
{
  static RenderState wantedState;
  return wantedState;
}

// NOLINTNEXTLINE(misc-no-recursion)
void RenderState::applyWantedState()
{
  getWantedState().apply();
}

RenderState RenderState::getDefaults()
{
  static bool initialized = false;

  static RenderState defaults;
  if(!initialized)
  {
    initialized = true;
    defaults.setCullFace(true);
    defaults.setDepthTest(true);
    defaults.setDepthWrite(true);
    defaults.setDepthClamp(false);
    defaults.setDepthFunction(api::DepthFunction::Less);
    for(uint32_t i = 0; i < IndexedCaps; ++i)
      defaults.setBlend(i, false);
    for(uint32_t i = 0; i < IndexedCaps; ++i)
      defaults.setBlendFactors(i, api::BlendingFactor::SrcAlpha, api::BlendingFactor::OneMinusSrcAlpha);
    defaults.setCullFaceSide(api::TriangleFace::Back);
    defaults.setFrontFace(api::FrontFaceDirection::Cw);
    defaults.setLineWidth(1.0f);
    defaults.setLineSmooth(true);
    defaults.setScissorTest(false);
    defaults.setScissorRegion({0, 0}, {0, 0});
    defaults.setPolygonOffsetFill(false);
    defaults.setPolygonOffset(0, 0);
  }
  return defaults;
}

void RenderState::reset()
{
  resetWantedState();
  getCurrentState(true);
}
} // namespace gl
