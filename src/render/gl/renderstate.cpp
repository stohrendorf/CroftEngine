#include "renderstate.h"

namespace render::gl
{
RenderState& RenderState::getCurrentState()
{
  static RenderState currentState;
  return currentState;
}

void RenderState::apply(const bool force) const
{
  // Update any state if...
  //   - it is forced
  //   - or it is explicitly set and different than the current state
#define RS_CHANGED(m) (force || (m.isInitialized() && m != getCurrentState().m))
  if(RS_CHANGED(m_blendEnabled))
  {
    if(m_blendEnabled.get())
    {
      GL_ASSERT(::gl::enable(::gl::EnableCap::Blend));
    }
    else
    {
      GL_ASSERT(::gl::disable(::gl::EnableCap::Blend));
    }
    getCurrentState().m_blendEnabled = m_blendEnabled;
  }
  if(RS_CHANGED(m_blendSrc) || RS_CHANGED(m_blendDst))
  {
    GL_ASSERT(::gl::blendFunc(m_blendSrc.get(), m_blendDst.get()));
    getCurrentState().m_blendSrc = m_blendSrc;
    getCurrentState().m_blendDst = m_blendDst;
  }
  if(RS_CHANGED(m_cullFaceEnabled))
  {
    if(m_cullFaceEnabled.get())
    {
      GL_ASSERT(::gl::enable(::gl::EnableCap::CullFace));
    }
    else
    {
      GL_ASSERT(::gl::disable(::gl::EnableCap::CullFace));
    }
    getCurrentState().m_cullFaceEnabled = m_cullFaceEnabled;
  }
  if(RS_CHANGED(m_cullFaceSide))
  {
    GL_ASSERT(::gl::cullFace(m_cullFaceSide.get()));
    getCurrentState().m_cullFaceSide = m_cullFaceSide;
  }
  if(RS_CHANGED(m_frontFace))
  {
    GL_ASSERT(::gl::frontFace(m_frontFace.get()));
    getCurrentState().m_frontFace = m_frontFace;
  }
  if(RS_CHANGED(m_lineWidth))
  {
    GL_ASSERT(::gl::lineWidth(m_lineWidth.get()));
    getCurrentState().m_lineWidth = m_lineWidth;
  }
  if(RS_CHANGED(m_lineSmooth))
  {
    if(m_lineSmooth.get())
    {
      GL_ASSERT(::gl::enable(::gl::EnableCap::LineSmooth));
    }
    else
    {
      GL_ASSERT(::gl::disable(::gl::EnableCap::LineSmooth));
    }
    getCurrentState().m_lineSmooth = m_lineSmooth;
  }
  if(RS_CHANGED(m_depthTestEnabled))
  {
    if(m_depthTestEnabled.get())
    {
      GL_ASSERT(::gl::enable(::gl::EnableCap::DepthTest));
    }
    else
    {
      GL_ASSERT(::gl::disable(::gl::EnableCap::DepthTest));
    }
    getCurrentState().m_depthTestEnabled = m_depthTestEnabled;
  }
  if(RS_CHANGED(m_depthWriteEnabled))
  {
    GL_ASSERT(::gl::depthMask(m_depthWriteEnabled.get()));
    getCurrentState().m_depthWriteEnabled = m_depthWriteEnabled;
  }
  if(RS_CHANGED(m_depthFunction))
  {
    GL_ASSERT(::gl::depthFunc(m_depthFunction.get()));
    getCurrentState().m_depthFunction = m_depthFunction;
  }
#undef RS_CHANGED
}

void RenderState::enableDepthWrite()
{
  // Internal method used by Renderer::clear() to restore depth writing before a
  // clear operation. This is necessary if the last code to draw before the
  // next frame leaves depth writing disabled.
  GL_ASSERT(::gl::depthMask(true));
  getCurrentState().m_depthWriteEnabled = true;
  GL_ASSERT(::gl::enable(::gl::EnableCap::DepthTest));
  getCurrentState().m_depthTestEnabled = true;
}

void RenderState::initDefaults()
{
  getCurrentState().m_cullFaceEnabled.setDefault();
  getCurrentState().m_depthTestEnabled.setDefault();
  getCurrentState().m_depthWriteEnabled.setDefault();
  getCurrentState().m_depthFunction.setDefault();
  getCurrentState().m_blendEnabled.setDefault();
  getCurrentState().m_blendSrc.setDefault();
  getCurrentState().m_blendDst.setDefault();
  getCurrentState().m_cullFaceSide.setDefault();
  getCurrentState().m_frontFace.setDefault();
  getCurrentState().m_lineWidth.setDefault();
  getCurrentState().m_lineSmooth.setDefault();
  getCurrentState().apply(true);
}

void RenderState::merge(const RenderState& other)
{
#define MERGE_OPT(n) n.merge(other.n)
  MERGE_OPT(m_cullFaceEnabled);
  MERGE_OPT(m_depthTestEnabled);
  MERGE_OPT(m_depthWriteEnabled);
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
} // namespace render::gl
