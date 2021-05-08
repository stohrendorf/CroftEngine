#include "renderstate.h"

#include "glassert.h"

namespace gl
{
inline RenderState& getCurrentState()
{
  static RenderState currentState{};
  static bool initialized = false;

  if(!initialized)
  {
    RenderState::resetWantedState();
    initialized = true;
    RenderState::applyWantedState();
  }
  return currentState;
}

void RenderState::apply(const bool force) const
{
  // Update any state if...
  //   - it is forced
  //   - or it is explicitly set and different than the current state
  // NOLINTNEXTLINE(bugprone-macro-parentheses)
#define RS_CHANGED(m) (force || (m.has_value() && m != getCurrentState().m))
  if(RS_CHANGED(m_viewport))
  {
    GL_ASSERT(api::viewport(0, 0, m_viewport->x, m_viewport->y));
    getCurrentState().m_viewport = m_viewport;
  }
  if(RS_CHANGED(m_program))
  {
    GL_ASSERT(api::useProgram(m_program.value()));
    getCurrentState().m_program = m_program;
  }
  if(RS_CHANGED(m_blendEnabled))
  {
    if(m_blendEnabled.value())
    {
      GL_ASSERT(api::enable(api::EnableCap::Blend));
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::Blend));
    }
    getCurrentState().m_blendEnabled = m_blendEnabled;
  }
  if(RS_CHANGED(m_blendSrc) || RS_CHANGED(m_blendDst))
  {
    GL_ASSERT(api::blendFunc(m_blendSrc.value(), m_blendDst.value()));
    getCurrentState().m_blendSrc = m_blendSrc;
    getCurrentState().m_blendDst = m_blendDst;
  }
  if(RS_CHANGED(m_cullFaceEnabled))
  {
    if(m_cullFaceEnabled.value())
    {
      GL_ASSERT(api::enable(api::EnableCap::CullFace));
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::CullFace));
    }
    getCurrentState().m_cullFaceEnabled = m_cullFaceEnabled;
  }
  if(RS_CHANGED(m_cullFaceSide))
  {
    GL_ASSERT(api::cullFace(m_cullFaceSide.value()));
    getCurrentState().m_cullFaceSide = m_cullFaceSide;
  }
  if(RS_CHANGED(m_frontFace))
  {
    GL_ASSERT(api::frontFace(m_frontFace.value()));
    getCurrentState().m_frontFace = m_frontFace;
  }
  if(RS_CHANGED(m_lineWidth))
  {
    GL_ASSERT(api::lineWidth(m_lineWidth.value()));
    getCurrentState().m_lineWidth = m_lineWidth;
  }
  if(RS_CHANGED(m_lineSmooth))
  {
    if(m_lineSmooth.value())
    {
      GL_ASSERT(api::enable(api::EnableCap::LineSmooth));
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::LineSmooth));
    }
    getCurrentState().m_lineSmooth = m_lineSmooth;
  }
  if(RS_CHANGED(m_depthTestEnabled))
  {
    if(m_depthTestEnabled.value())
    {
      GL_ASSERT(api::enable(api::EnableCap::DepthTest));
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::DepthTest));
    }
    getCurrentState().m_depthTestEnabled = m_depthTestEnabled;
  }
  if(RS_CHANGED(m_depthWriteEnabled))
  {
    GL_ASSERT(api::depthMask(m_depthWriteEnabled.value()));
    getCurrentState().m_depthWriteEnabled = m_depthWriteEnabled;
  }
  if(RS_CHANGED(m_depthClampEnabled))
  {
    if(m_depthClampEnabled.value())
    {
      GL_ASSERT(api::enable(api::EnableCap::DepthClamp));
    }
    else
    {
      GL_ASSERT(api::disable(api::EnableCap::DepthClamp));
    }
    getCurrentState().m_depthClampEnabled = m_depthClampEnabled;
  }
  if(RS_CHANGED(m_depthFunction))
  {
    GL_ASSERT(api::depthFunc(m_depthFunction.value()));
    getCurrentState().m_depthFunction = m_depthFunction;
  }
#undef RS_CHANGED
}

void RenderState::merge(const RenderState& other)
{
#define MERGE_OPT(n)                                               \
  if(other.n.has_value()) /* NOLINT(bugprone-macro-parentheses) */ \
  n = other.n             /* NOLINT(bugprone-macro-parentheses) */
  MERGE_OPT(m_cullFaceEnabled);
  MERGE_OPT(m_depthTestEnabled);
  MERGE_OPT(m_depthWriteEnabled);
  MERGE_OPT(m_depthClampEnabled);
  MERGE_OPT(m_depthFunction);
  MERGE_OPT(m_blendEnabled);
  MERGE_OPT(m_blendSrc);
  MERGE_OPT(m_blendDst);
  MERGE_OPT(m_cullFaceSide);
  MERGE_OPT(m_frontFace);
  MERGE_OPT(m_lineWidth);
  MERGE_OPT(m_lineSmooth);
#undef MERGE_OPT
}

RenderState& RenderState::getWantedState()
{
  static RenderState wantedState;
  return wantedState;
}

void RenderState::applyWantedState()
{
  getWantedState().apply();
}

RenderState RenderState::getDefaults()
{
  static bool initialized = false;

  RenderState defaults;
  if(!initialized)
  {
    initialized = true;
    defaults.setCullFace(true);
    defaults.setDepthTest(true);
    defaults.setDepthWrite(true);
    defaults.setDepthClamp(false);
    defaults.setDepthFunction(api::DepthFunction::Less);
    defaults.setBlend(true);
    defaults.setBlendSrc(api::BlendingFactor::SrcAlpha);
    defaults.setBlendDst(api::BlendingFactor::OneMinusSrcAlpha);
    defaults.setCullFaceSide(api::CullFaceMode::Back);
    defaults.setFrontFace(api::FrontFaceDirection::Cw);
    defaults.setLineWidth(1.0f);
    defaults.setLineSmooth(true);
  }
  return defaults;
}
} // namespace gl
