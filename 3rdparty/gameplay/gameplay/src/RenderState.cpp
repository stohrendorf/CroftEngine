#include "Base.h"
#include "RenderState.h"
#include "Node.h"

// Render state override bits
#define RS_BLEND 1
#define RS_BLEND_FUNC 2
#define RS_CULL_FACE 4
#define RS_DEPTH_TEST 8
#define RS_DEPTH_WRITE 16
#define RS_DEPTH_FUNC 32
#define RS_CULL_FACE_SIDE 64
#define RS_FRONT_FACE 2048

namespace gameplay
{
    std::shared_ptr<RenderState::StateBlock> RenderState::StateBlock::m_defaultState = nullptr;

    void RenderState::initialize()
    {
        if( StateBlock::m_defaultState == nullptr )
        {
            StateBlock::m_defaultState = std::make_shared<StateBlock>();
        }
    }

    void RenderState::finalize()
    {
        StateBlock::m_defaultState.reset();
    }

    const std::shared_ptr<RenderState::StateBlock>& RenderState::getStateBlock() const
    {
        if( m_state == nullptr )
        {
            m_state = std::make_shared<StateBlock>();
        }

        return m_state;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::initStateBlockDefaults()
    {
        getStateBlock(); // alloc if not done yet
        m_state->setDepthTest( true );
        m_state->setDepthFunction( GL_LESS );
        m_state->setCullFace( true );
        m_state->setFrontFace( GL_CW );
        m_state->setBlend( true );
        m_state->setBlendSrc( GL_SRC_ALPHA );
        m_state->setBlendDst( GL_ONE_MINUS_SRC_ALPHA );
    }

    void RenderState::bind()
    {
        StateBlock::restore( m_state ? m_state->m_bits : 0 );

        if( m_state )
        {
            m_state->bindNoRestore();
        }
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
        restore( m_bits );

        bindNoRestore();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::StateBlock::bindNoRestore()
    {
        BOOST_ASSERT( m_defaultState );

        // Update any state that differs from m_defaultState and flip m_defaultState bits
        if( (m_bits & RS_BLEND) && (m_blendEnabled != m_defaultState->m_blendEnabled) )
        {
            if( m_blendEnabled )
            {
                GL_ASSERT( glEnable( GL_BLEND ) );
            }
            else
            {
                GL_ASSERT( glDisable( GL_BLEND ) );
            }
            m_defaultState->m_blendEnabled = m_blendEnabled;
        }
        if( (m_bits & RS_BLEND_FUNC)
            && (m_blendSrc != m_defaultState->m_blendSrc || m_blendDst != m_defaultState->m_blendDst) )
        {
            GL_ASSERT( glBlendFunc( m_blendSrc, m_blendDst ) );
            m_defaultState->m_blendSrc = m_blendSrc;
            m_defaultState->m_blendDst = m_blendDst;
        }
        if( (m_bits & RS_CULL_FACE) && (m_cullFaceEnabled != m_defaultState->m_cullFaceEnabled) )
        {
            if( m_cullFaceEnabled )
            {
                GL_ASSERT( glEnable( GL_CULL_FACE ) );
            }
            else
            {
                GL_ASSERT( glDisable( GL_CULL_FACE ) );
            }
            m_defaultState->m_cullFaceEnabled = m_cullFaceEnabled;
        }
        if( (m_bits & RS_CULL_FACE_SIDE) && (m_cullFaceSide != m_defaultState->m_cullFaceSide) )
        {
            GL_ASSERT( glCullFace( m_cullFaceSide ) );
            m_defaultState->m_cullFaceSide = m_cullFaceSide;
        }
        if( (m_bits & RS_FRONT_FACE) && (m_frontFace != m_defaultState->m_frontFace) )
        {
            GL_ASSERT( glFrontFace( m_frontFace ) );
            m_defaultState->m_frontFace = m_frontFace;
        }
        if( (m_bits & RS_DEPTH_TEST) && (m_depthTestEnabled != m_defaultState->m_depthTestEnabled) )
        {
            if( m_depthTestEnabled )
            {
                GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
            }
            else
            {
                GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
            }
            m_defaultState->m_depthTestEnabled = m_depthTestEnabled;
        }
        if( (m_bits & RS_DEPTH_WRITE) && (m_depthWriteEnabled != m_defaultState->m_depthWriteEnabled) )
        {
            GL_ASSERT( glDepthMask( m_depthWriteEnabled ? GL_TRUE : GL_FALSE ) );
            m_defaultState->m_depthWriteEnabled = m_depthWriteEnabled;
        }
        if( (m_bits & RS_DEPTH_FUNC) && (m_depthFunction != m_defaultState->m_depthFunction) )
        {
            GL_ASSERT( glDepthFunc( m_depthFunction ) );
            m_defaultState->m_depthFunction = m_depthFunction;
        }

        m_defaultState->m_bits |= m_bits;
    }

    void RenderState::StateBlock::restore(long stateOverrideBits)
    {
        BOOST_ASSERT( m_defaultState );

        // If there is no state to restore (i.e. no non-default state), do nothing.
        if( m_defaultState->m_bits == 0 )
        {
            return;
        }

        // Restore any state that is not overridden and is not default
        if( !(stateOverrideBits & RS_BLEND) && (m_defaultState->m_bits & RS_BLEND) )
        {
            GL_ASSERT( glDisable( GL_BLEND ) );
            m_defaultState->m_bits &= ~RS_BLEND;
            m_defaultState->m_blendEnabled = false;
        }
        if( !(stateOverrideBits & RS_BLEND_FUNC) && (m_defaultState->m_bits & RS_BLEND_FUNC) )
        {
            GL_ASSERT( glBlendFunc( GL_ONE, GL_ZERO ) );
            m_defaultState->m_bits &= ~RS_BLEND_FUNC;
            m_defaultState->m_blendSrc = GL_ONE;
            m_defaultState->m_blendDst = GL_ZERO;
        }
        if( !(stateOverrideBits & RS_CULL_FACE) && (m_defaultState->m_bits & RS_CULL_FACE) )
        {
            GL_ASSERT( glDisable( GL_CULL_FACE ) );
            m_defaultState->m_bits &= ~RS_CULL_FACE;
            m_defaultState->m_cullFaceEnabled = false;
        }
        if( !(stateOverrideBits & RS_CULL_FACE_SIDE) && (m_defaultState->m_bits & RS_CULL_FACE_SIDE) )
        {
            GL_ASSERT( glCullFace( GL_BACK ) );
            m_defaultState->m_bits &= ~RS_CULL_FACE_SIDE;
            m_defaultState->m_cullFaceSide = GL_BACK;
        }
        if( !(stateOverrideBits & RS_FRONT_FACE) && (m_defaultState->m_bits & RS_FRONT_FACE) )
        {
            GL_ASSERT( glFrontFace( GL_CCW ) );
            m_defaultState->m_bits &= ~RS_FRONT_FACE;
            m_defaultState->m_frontFace = GL_CCW;
        }
        if( !(stateOverrideBits & RS_DEPTH_TEST) && (m_defaultState->m_bits & RS_DEPTH_TEST) )
        {
            GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
            m_defaultState->m_bits &= ~RS_DEPTH_TEST;
            m_defaultState->m_depthTestEnabled = false;
        }
        if( !(stateOverrideBits & RS_DEPTH_WRITE) && (m_defaultState->m_bits & RS_DEPTH_WRITE) )
        {
            GL_ASSERT( glDepthMask( GL_TRUE ) );
            m_defaultState->m_bits &= ~RS_DEPTH_WRITE;
            m_defaultState->m_depthWriteEnabled = true;
        }
        if( !(stateOverrideBits & RS_DEPTH_FUNC) && (m_defaultState->m_bits & RS_DEPTH_FUNC) )
        {
            GL_ASSERT( glDepthFunc( GL_LESS ) );
            m_defaultState->m_bits &= ~RS_DEPTH_FUNC;
            m_defaultState->m_depthFunction = GL_LESS;
        }
    }

    void RenderState::StateBlock::enableDepthWrite()
    {
        BOOST_ASSERT( m_defaultState );

        // Internal method used by Game::clear() to restore depth writing before a
        // clear operation. This is necessary if the last code to draw before the
        // next frame leaves depth writing disabled.
        if( !m_defaultState->m_depthWriteEnabled )
        {
            GL_ASSERT( glDepthMask( GL_TRUE ) );
            m_defaultState->m_bits &= ~RS_DEPTH_WRITE;
            m_defaultState->m_depthWriteEnabled = true;
        }
    }

    void RenderState::StateBlock::setBlend(bool enabled)
    {
        m_blendEnabled = enabled;
        if( !enabled )
        {
            m_bits &= ~RS_BLEND;
        }
        else
        {
            m_bits |= RS_BLEND;
        }
    }

    void RenderState::StateBlock::setBlendSrc(GLenum blend)
    {
        m_blendSrc = blend;
        if( m_blendSrc == GL_ONE && m_blendDst == GL_ZERO )
        {
            // Default blend func
            m_bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            m_bits |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setBlendDst(GLenum blend)
    {
        m_blendDst = blend;
        if( m_blendSrc == GL_ONE && m_blendDst == GL_ZERO )
        {
            // Default blend func
            m_bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            m_bits |= RS_BLEND_FUNC;
        }
    }

    void RenderState::StateBlock::setCullFace(bool enabled)
    {
        m_cullFaceEnabled = enabled;
        if( !enabled )
        {
            m_bits &= ~RS_CULL_FACE;
        }
        else
        {
            m_bits |= RS_CULL_FACE;
        }
    }

    void RenderState::StateBlock::setCullFaceSide(GLenum side)
    {
        m_cullFaceSide = side;
        if( m_cullFaceSide == GL_BACK )
        {
            // Default cull side
            m_bits &= ~RS_CULL_FACE_SIDE;
        }
        else
        {
            m_bits |= RS_CULL_FACE_SIDE;
        }
    }

    void RenderState::StateBlock::setFrontFace(GLenum winding)
    {
        m_frontFace = winding;
        if( m_frontFace == GL_CCW )
        {
            // Default front face
            m_bits &= ~RS_FRONT_FACE;
        }
        else
        {
            m_bits |= RS_FRONT_FACE;
        }
    }

    void RenderState::StateBlock::setDepthTest(bool enabled)
    {
        m_depthTestEnabled = enabled;
        if( !enabled )
        {
            m_bits &= ~RS_DEPTH_TEST;
        }
        else
        {
            m_bits |= RS_DEPTH_TEST;
        }
    }

    void RenderState::StateBlock::setDepthWrite(bool enabled)
    {
        m_depthWriteEnabled = enabled;
        if( enabled )
        {
            m_bits &= ~RS_DEPTH_WRITE;
        }
        else
        {
            m_bits |= RS_DEPTH_WRITE;
        }
    }

    void RenderState::StateBlock::setDepthFunction(GLenum func)
    {
        m_depthFunction = func;
        if( m_depthFunction == GL_LESS )
        {
            // Default depth function
            m_bits &= ~RS_DEPTH_FUNC;
        }
        else
        {
            m_bits |= RS_DEPTH_FUNC;
        }
    }
}
