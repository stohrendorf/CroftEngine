#include "renderstate.h"

namespace render
{
namespace gl
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
    if( force || (m_blendEnabled.isInitialized() && m_blendEnabled != getCurrentState().m_blendEnabled) )
    {
        if( m_blendEnabled.get() )
        {
            GL_ASSERT( glEnable( GL_BLEND ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_BLEND ) );
        }
        getCurrentState().m_blendEnabled = m_blendEnabled;
    }
    if( force || ((m_blendSrc.isInitialized() || m_blendDst.isInitialized())
                  && (m_blendSrc != getCurrentState().m_blendSrc || m_blendDst != getCurrentState().m_blendDst)) )
    {
        GL_ASSERT( glBlendFunc( m_blendSrc.get(), m_blendDst.get() ) );
        getCurrentState().m_blendSrc = m_blendSrc;
        getCurrentState().m_blendDst = m_blendDst;
    }
    if( force || (m_cullFaceEnabled.isInitialized() && m_cullFaceEnabled != getCurrentState().m_cullFaceEnabled) )
    {
        if( m_cullFaceEnabled.get() )
        {
            GL_ASSERT( glEnable( GL_CULL_FACE ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_CULL_FACE ) );
        }
        getCurrentState().m_cullFaceEnabled = m_cullFaceEnabled;
    }
    if( force || (m_cullFaceSide.isInitialized() && (m_cullFaceSide != getCurrentState().m_cullFaceSide)) )
    {
        GL_ASSERT( glCullFace( m_cullFaceSide.get() ) );
        getCurrentState().m_cullFaceSide = m_cullFaceSide;
    }
    if( force || (m_frontFace.isInitialized() && m_frontFace != getCurrentState().m_frontFace) )
    {
        GL_ASSERT( glFrontFace( m_frontFace.get() ) );
        getCurrentState().m_frontFace = m_frontFace;
    }
    if( force || (m_lineWidth.isInitialized() && m_lineWidth != getCurrentState().m_lineWidth) )
    {
        GL_ASSERT( glLineWidth( m_lineWidth.get() ) );
        getCurrentState().m_lineWidth = m_lineWidth;
    }
    if( force || (m_lineSmooth.isInitialized() && (m_lineSmooth != getCurrentState().m_lineSmooth)) )
    {
        if( m_lineSmooth.get() )
        {
            GL_ASSERT( glEnable( GL_LINE_SMOOTH ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_LINE_SMOOTH ) );
        }
        getCurrentState().m_lineSmooth = m_lineSmooth;
    }
    if( force || (m_depthTestEnabled.isInitialized() && (m_depthTestEnabled != getCurrentState().m_depthTestEnabled)) )
    {
        if( m_depthTestEnabled.get() )
        {
            GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
        }
        getCurrentState().m_depthTestEnabled = m_depthTestEnabled;
    }
    if( force || (m_depthWriteEnabled.isInitialized() && (m_depthWriteEnabled != getCurrentState().m_depthWriteEnabled)) )
    {
        GL_ASSERT( glDepthMask( m_depthWriteEnabled.get() ? GL_TRUE : GL_FALSE ) );
        getCurrentState().m_depthWriteEnabled = m_depthWriteEnabled;
    }
    if( force || (m_depthFunction.isInitialized() && (m_depthFunction != getCurrentState().m_depthFunction)) )
    {
        GL_ASSERT( glDepthFunc( m_depthFunction.get() ) );
        getCurrentState().m_depthFunction = m_depthFunction;
    }
}

void RenderState::enableDepthWrite()
{
    // Internal method used by Game::clear() to restore depth writing before a
    // clear operation. This is necessary if the last code to draw before the
    // next frame leaves depth writing disabled.
    GL_ASSERT( glDepthMask( GL_TRUE ) );
    getCurrentState().m_depthWriteEnabled = true;
    GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
    getCurrentState().m_depthTestEnabled = true;
}

void RenderState::setBlend(const bool enabled)
{
    m_blendEnabled = enabled;
}

void RenderState::setBlendSrc(const GLenum blend)
{
    m_blendSrc = blend;
}

void RenderState::setBlendDst(const GLenum blend)
{
    m_blendDst = blend;
}

void RenderState::setCullFace(const bool enabled)
{
    m_cullFaceEnabled = enabled;
}

void RenderState::setCullFaceSide(const GLenum side)
{
    m_cullFaceSide = side;
}

void RenderState::setFrontFace(const GLenum winding)
{
    m_frontFace = winding;
}

void RenderState::setDepthTest(const bool enabled)
{
    m_depthTestEnabled = enabled;
}

void RenderState::setDepthWrite(const bool enabled)
{
    m_depthWriteEnabled = enabled;
}

void RenderState::setDepthFunction(const GLenum func)
{
    m_depthFunction = func;
}

void RenderState::setLineWidth(GLfloat width)
{
    m_lineWidth = width;
}

void RenderState::setLineSmooth(bool enabled)
{
    m_lineSmooth = enabled;
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
    getCurrentState().apply( true );
}

void RenderState::merge(const RenderState& other)
{
#define MERGE_OPT(n) n.merge(other.n)
    MERGE_OPT( m_cullFaceEnabled );
    MERGE_OPT( m_depthTestEnabled );
    MERGE_OPT( m_depthWriteEnabled );
    MERGE_OPT( m_depthFunction );
    MERGE_OPT( m_blendEnabled );
    MERGE_OPT( m_blendSrc );
    MERGE_OPT( m_blendDst );
    MERGE_OPT( m_cullFaceSide );
    MERGE_OPT( m_frontFace );
    MERGE_OPT( m_lineWidth );
    MERGE_OPT( m_lineSmooth );
#undef MERGE_OPT
}
}
}
