#include "Base.h"
#include "RenderState.h"
#include "Node.h"

namespace gameplay
{
RenderState::StateBlock RenderState::StateBlock::m_currentState;

RenderState::StateBlock& RenderState::getStateBlock()
{
    return m_state;
}

void RenderState::bind()
{
    m_state.bind();
}

RenderState::StateBlock::StateBlock() = default;

RenderState::StateBlock::~StateBlock() = default;

void RenderState::StateBlock::bind()
{
    // When the public bind() is called with no RenderState object passed in,
    // we assume we are being called to bind the state of a single StateBlock,
    // irrespective of whether it belongs to a hierarchy of RenderStates.
    // Therefore, we call restore() here with only this StateBlock's override
    // bits to restore state before applying the new state.
    restore();
    bindNoRestore();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void RenderState::StateBlock::bindNoRestore()
{
    // Update any state that differs from m_currentState
    if( m_blendEnabled.is_initialized() && m_blendEnabled != m_currentState.m_blendEnabled )
    {
        if( *m_blendEnabled )
        {
            GL_ASSERT( glEnable( GL_BLEND ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_BLEND ) );
        }
        m_currentState.m_blendEnabled = m_blendEnabled;
    }
    if( (m_blendSrc.is_initialized() || m_blendDst.is_initialized())
        && (m_blendSrc != m_currentState.m_blendSrc || m_blendDst != m_currentState.m_blendDst) )
    {
        GL_ASSERT( glBlendFunc( m_blendSrc.get_value_or( GL_SRC_ALPHA ),
                                m_blendDst.get_value_or( GL_ONE_MINUS_SRC_ALPHA ) ) );
        m_currentState.m_blendSrc = m_blendSrc;
        m_currentState.m_blendDst = m_blendDst;
    }
    if( m_cullFaceEnabled.is_initialized() && m_cullFaceEnabled != m_currentState.m_cullFaceEnabled )
    {
        if( *m_cullFaceEnabled )
        {
            GL_ASSERT( glEnable( GL_CULL_FACE ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_CULL_FACE ) );
        }
        m_currentState.m_cullFaceEnabled = m_cullFaceEnabled;
    }
    if( m_cullFaceSide.is_initialized() && (m_cullFaceSide != m_currentState.m_cullFaceSide) )
    {
        GL_ASSERT( glCullFace( *m_cullFaceSide ) );
        m_currentState.m_cullFaceSide = m_cullFaceSide;
    }
    if( m_frontFace.is_initialized() && m_frontFace != m_currentState.m_frontFace )
    {
        GL_ASSERT( glFrontFace( *m_frontFace ) );
        m_currentState.m_frontFace = m_frontFace;
    }
    if( m_depthTestEnabled.is_initialized() && (m_depthTestEnabled != m_currentState.m_depthTestEnabled) )
    {
        if( *m_depthTestEnabled )
        {
            GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
        }
        else
        {
            GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
        }
        m_currentState.m_depthTestEnabled = m_depthTestEnabled;
    }
    if( m_depthWriteEnabled.is_initialized() && (m_depthWriteEnabled != m_currentState.m_depthWriteEnabled) )
    {
        GL_ASSERT( glDepthMask( *m_depthWriteEnabled ? GL_TRUE : GL_FALSE ) );
        m_currentState.m_depthWriteEnabled = m_depthWriteEnabled;
    }
    if( m_depthFunction.is_initialized() && (m_depthFunction != m_currentState.m_depthFunction) )
    {
        GL_ASSERT( glDepthFunc( *m_depthFunction ) );
        m_currentState.m_depthFunction = m_depthFunction;
    }
}

void RenderState::StateBlock::restore(bool forceDefaults)
{
    // Restore any state that is not overridden and is not default
    if( forceDefaults || !m_blendEnabled.get_value_or( true ) )
    {
        GL_ASSERT( glEnable( GL_BLEND ) );
        m_currentState.m_blendEnabled.reset();
    }
    if( forceDefaults || m_blendSrc.get_value_or( GL_SRC_ALPHA ) != GL_SRC_ALPHA
        || m_blendDst.get_value_or( GL_ONE_MINUS_SRC_ALPHA ) != GL_ONE_MINUS_SRC_ALPHA )
    {
        GL_ASSERT( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
        m_currentState.m_blendSrc.reset();
        m_currentState.m_blendDst.reset();
    }
    if( forceDefaults || !m_cullFaceEnabled.get_value_or( true ) )
    {
        GL_ASSERT( glEnable( GL_CULL_FACE ) );
        m_currentState.m_cullFaceEnabled.reset();
    }
    if( forceDefaults || m_cullFaceSide.get_value_or( GL_BACK ) != GL_BACK )
    {
        GL_ASSERT( glCullFace( GL_BACK ) );
        m_currentState.m_cullFaceSide.reset();
    }
    if( forceDefaults || m_frontFace.get_value_or( GL_CW ) != GL_CW )
    {
        GL_ASSERT( glFrontFace( GL_CW ) );
        m_currentState.m_frontFace.reset();
    }
    if( forceDefaults || !m_depthTestEnabled.get_value_or( true ) )
    {
        GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
        m_currentState.m_depthTestEnabled.reset();
    }
    if( forceDefaults || !m_depthWriteEnabled.get_value_or( true ) )
    {
        GL_ASSERT( glDepthMask( GL_TRUE ) );
        m_currentState.m_depthWriteEnabled.reset();
    }
    if( forceDefaults || m_depthFunction.get_value_or( GL_LESS ) != GL_LESS )
    {
        GL_ASSERT( glDepthFunc( GL_LESS ) );
        m_currentState.m_depthFunction.reset();
    }
}

void RenderState::StateBlock::enableDepthWrite()
{
    // Internal method used by Game::clear() to restore depth writing before a
    // clear operation. This is necessary if the last code to draw before the
    // next frame leaves depth writing disabled.
    GL_ASSERT( glDepthMask( GL_TRUE ) );
    m_currentState.m_depthWriteEnabled.reset();
}

void RenderState::StateBlock::setBlend(bool enabled)
{
    m_blendEnabled = enabled;
    if( enabled )
    {
        m_blendEnabled.reset();
    }
}

void RenderState::StateBlock::setBlendSrc(GLenum blend)
{
    m_blendSrc = blend;
    if( blend == GL_SRC_ALPHA )
    {
        m_blendSrc.reset();
    }
}

void RenderState::StateBlock::setBlendDst(GLenum blend)
{
    m_blendDst = blend;
    if( blend == GL_ONE_MINUS_SRC_ALPHA )
    {
        m_blendDst.reset();
    }
}

void RenderState::StateBlock::setCullFace(bool enabled)
{
    m_cullFaceEnabled = enabled;
    if( enabled )
    {
        m_cullFaceEnabled.reset();
    }
}

void RenderState::StateBlock::setCullFaceSide(GLenum side)
{
    m_cullFaceSide = side;
    if( side == GL_BACK )
    {
        m_cullFaceSide.reset();
    }
}

void RenderState::StateBlock::setFrontFace(GLenum winding)
{
    m_frontFace = winding;
    if( winding == GL_CW )
    {
        m_frontFace.reset();
    }
}

void RenderState::StateBlock::setDepthTest(bool enabled)
{
    m_depthTestEnabled = enabled;
    if( enabled )
    {
        m_depthTestEnabled.reset();
    }
}

void RenderState::StateBlock::setDepthWrite(bool enabled)
{
    m_depthWriteEnabled = enabled;
    if( enabled )
    {
        m_depthWriteEnabled.reset();
    }
}

void RenderState::StateBlock::setDepthFunction(GLenum func)
{
    m_depthFunction = func;
    if( func == GL_LESS )
    {
        m_depthFunction.reset();
    }
}
}
