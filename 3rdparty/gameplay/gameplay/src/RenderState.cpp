#include "Base.h"
#include "RenderState.h"
#include "Node.h"
#include "Scene.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>

// Render state override bits
#define RS_BLEND 1
#define RS_BLEND_FUNC 2
#define RS_CULL_FACE 4
#define RS_DEPTH_TEST 8
#define RS_DEPTH_WRITE 16
#define RS_DEPTH_FUNC 32
#define RS_CULL_FACE_SIDE 64
#define RS_STENCIL_TEST 128
#define RS_STENCIL_WRITE 256
#define RS_STENCIL_FUNC 512
#define RS_STENCIL_OP 1024
#define RS_FRONT_FACE 2048

#define RS_ALL_ONES 0xFFFFFFFF


namespace gameplay
{
    std::shared_ptr<RenderState::StateBlock> RenderState::StateBlock::_defaultState = nullptr;
    std::vector<RenderState::AutoBindingResolver*> RenderState::_customAutoBindingResolvers;


    RenderState::RenderState()
        : _nodeBinding(nullptr)
        , _state(nullptr)
        , _parent(nullptr)
    {
    }


    RenderState::~RenderState() = default;


    void RenderState::initialize()
    {
        if( StateBlock::_defaultState == nullptr )
        {
            StateBlock::_defaultState = std::make_shared<StateBlock>();
        }
    }


    void RenderState::finalize()
    {
        StateBlock::_defaultState.reset();
    }


    std::shared_ptr<MaterialParameter> RenderState::getParameter(const std::string& name) const
    {
        // Search for an existing parameter with this name.
        for( const auto& param : _parameters )
        {
            BOOST_ASSERT(param);
            if( param->getName() == name )
            {
                return param;
            }
        }

        // Create a new parameter and store it in our list.
        auto param = std::make_shared<MaterialParameter>(name);
        _parameters.push_back(param);

        return param;
    }


    size_t RenderState::getParameterCount() const
    {
        return _parameters.size();
    }


    const std::shared_ptr<MaterialParameter>& RenderState::getParameterByIndex(size_t index) const
    {
        return _parameters[index];
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::addParameter(const std::shared_ptr<MaterialParameter>& param)
    {
        _parameters.push_back(param);
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::removeParameter(const char* name)
    {
        for( size_t i = 0, count = _parameters.size(); i < count; ++i )
        {
            auto p = _parameters[i];
            if( p->m_name == name )
            {
                _parameters.erase(_parameters.begin() + i);
                break;
            }
        }
    }


    void RenderState::setParameterAutoBinding(const std::string& name, AutoBinding autoBinding)
    {
        // Add/update an auto-binding
        _autoBindings[name] = autoBinding;

        // If we already have a node binding set, pass it to our handler now
        if( _nodeBinding )
        {
            applyAutoBinding(name, autoBinding);
        }
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::setStateBlock(const std::shared_ptr<StateBlock>& state)
    {
        _state = state;
    }


    const std::shared_ptr<RenderState::StateBlock>& RenderState::getStateBlock() const
    {
        if( _state == nullptr )
        {
            _state = std::make_shared<StateBlock>();
            _state->setDepthTest(true);
            _state->setDepthFunction(DEPTH_LESS);
            _state->setCullFace(false);
            _state->setBlend(true);
            _state->setBlendSrc(BLEND_SRC_ALPHA);
            _state->setBlendDst(BLEND_ONE_MINUS_SRC_ALPHA);
        }

        return _state;
    }


    void RenderState::setNodeBinding(Node* node)
    {
        if( _nodeBinding == node )
            return;
        _nodeBinding = node;

        if( !_nodeBinding )
            return;

        for( const auto& binding : _autoBindings )
        {
            applyAutoBinding(binding.first, binding.second);
        }
    }


    void RenderState::applyAutoBinding(const std::string& uniformName, RenderState::AutoBinding autoBinding)
    {
        BOOST_ASSERT(_nodeBinding);

        auto param = getParameter(uniformName);
        BOOST_ASSERT(param);

        bool bound = false;

        // First attempt to resolve the binding using custom registered resolvers.
        for( size_t i = 0, count = _customAutoBindingResolvers.size(); i < count; ++i )
        {
            if( _customAutoBindingResolvers[i]->resolveAutoBinding(autoBinding, _nodeBinding, param) )
            {
                // Handled by custom auto binding resolver
                bound = true;
                break;
            }
        }

        // Perform built-in resolution
        if( bound )
            return;

        switch( autoBinding )
        {
            case WORLD_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetWorldMatrix);
                break;
            case VIEW_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetViewMatrix);
                break;
            case PROJECTION_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetProjectionMatrix);
                break;
            case WORLD_VIEW_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetWorldViewMatrix);
                break;
            case VIEW_PROJECTION_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetViewProjectionMatrix);
                break;
            case WORLD_VIEW_PROJECTION_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetWorldViewProjectionMatrix);
                break;
            case INVERSE_TRANSPOSE_WORLD_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetInverseTransposeWorldMatrix);
                break;
            case INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX:
                param->bindValue(this, &RenderState::autoBindingGetInverseTransposeWorldViewMatrix);
                break;
            case CAMERA_WORLD_POSITION:
                param->bindValue(this, &RenderState::autoBindingGetCameraWorldPosition);
                break;
            case SCENE_AMBIENT_COLOR:
                param->bindValue(this, &RenderState::autoBindingGetAmbientColor);
                break;
            default:
                BOOST_LOG_TRIVIAL(warning) << "Unsupported auto binding type (" << autoBinding << ").";
                break;
        }
    }


    const glm::mat4& RenderState::autoBindingGetWorldMatrix() const
    {
        static const glm::mat4 identity{ 1.0f };
        return _nodeBinding ? _nodeBinding->getWorldMatrix() : identity;
    }


    const glm::mat4& RenderState::autoBindingGetViewMatrix() const
    {
        static const glm::mat4 identity{ 1.0f };
        return _nodeBinding ? _nodeBinding->getViewMatrix() : identity;
    }


    const glm::mat4& RenderState::autoBindingGetProjectionMatrix() const
    {
        static const glm::mat4 identity{ 1.0f };
        return _nodeBinding ? _nodeBinding->getProjectionMatrix() : identity;
    }


    glm::mat4 RenderState::autoBindingGetWorldViewMatrix() const
    {
        return _nodeBinding ? _nodeBinding->getWorldViewMatrix() : glm::mat4{ 1.0f };
    }


    const glm::mat4& RenderState::autoBindingGetViewProjectionMatrix() const
    {
        static const glm::mat4 identity{ 1.0f };
        return _nodeBinding ? _nodeBinding->getViewProjectionMatrix() : identity;
    }


    glm::mat4 RenderState::autoBindingGetWorldViewProjectionMatrix() const
    {
        return _nodeBinding ? _nodeBinding->getWorldViewProjectionMatrix() : glm::mat4{ 1.0f };
    }


    glm::mat4 RenderState::autoBindingGetInverseTransposeWorldMatrix() const
    {
        return _nodeBinding ? _nodeBinding->getInverseTransposeWorldMatrix() : glm::mat4{ 1.0f };
    }


    glm::mat4 RenderState::autoBindingGetInverseTransposeWorldViewMatrix() const
    {
        return _nodeBinding ? _nodeBinding->getInverseTransposeWorldViewMatrix() : glm::mat4{ 1.0f };
    }


    glm::vec3 RenderState::autoBindingGetCameraWorldPosition() const
    {
        static const glm::vec3 zero{ 0, 0, 0 };
        return _nodeBinding ? _nodeBinding->getActiveCameraTranslationWorld() : zero;
    }


    const glm::vec3& RenderState::autoBindingGetAmbientColor() const
    {
        static const glm::vec3 zero{ 0, 0, 0 };

        Scene* scene = _nodeBinding ? _nodeBinding->getScene() : nullptr;
        return scene ? scene->getAmbientColor() : zero;
    }


    void RenderState::bind(Material* material)
    {
        BOOST_ASSERT(material);

        // Get the combined modified state bits for our RenderState hierarchy.
        long stateOverrideBits = _state ? _state->_bits : 0;
        auto rs = _parent;
        while( rs )
        {
            if( rs->_state )
            {
                stateOverrideBits |= rs->_state->_bits;
            }
            rs = rs->_parent;
        }

        // Restore renderer state to its default, except for explicitly specified states
        StateBlock::restore(stateOverrideBits);

        // Apply parameter bindings and renderer state for the entire hierarchy, top-down.
        rs = nullptr;
        auto shader = material->getShaderProgram();
        while( (rs = getTopmost(rs)) )
        {
            for( const auto& param : rs->_parameters )
            {
                BOOST_ASSERT(param);
                param->bind(shader);
            }

            if( rs->_state )
            {
                rs->_state->bindNoRestore();
            }
        }
    }


    RenderState* RenderState::getTopmost(const RenderState* below)
    {
        RenderState* rs = this;
        if( rs == below )
        {
            // Nothing below ourself.
            return nullptr;
        }

        while( rs )
        {
            if( rs->_parent == below || rs->_parent == nullptr )
            {
                // Stop traversing up here.
                return rs;
            }
            rs = rs->_parent;
        }

        return nullptr;
    }


    RenderState::StateBlock::StateBlock()
        : _stencilWrite(RS_ALL_ONES)
        , _stencilFunctionMask(RS_ALL_ONES)
    {
    }


    RenderState::StateBlock::~StateBlock() = default;


    void RenderState::StateBlock::bind()
    {
        // When the public bind() is called with no RenderState object passed in,
        // we assume we are being called to bind the state of a single StateBlock,
        // irrespective of whether it belongs to a hierarchy of RenderStates.
        // Therefore, we call restore() here with only this StateBlock's override
        // bits to restore state before applying the new state.
        StateBlock::restore(_bits);

        bindNoRestore();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void RenderState::StateBlock::bindNoRestore()
    {
        BOOST_ASSERT(_defaultState);

        // Update any state that differs from _defaultState and flip _defaultState bits
        if( (_bits & RS_BLEND) && (_blendEnabled != _defaultState->_blendEnabled) )
        {
            if( _blendEnabled )
                GL_ASSERT( glEnable(GL_BLEND) );
            else
                GL_ASSERT( glDisable(GL_BLEND) );
            _defaultState->_blendEnabled = _blendEnabled;
        }
        if( (_bits & RS_BLEND_FUNC) && (_blendSrc != _defaultState->_blendSrc || _blendDst != _defaultState->_blendDst) )
        {
            GL_ASSERT( glBlendFunc(static_cast<GLenum>(_blendSrc), static_cast<GLenum>(_blendDst)) );
            _defaultState->_blendSrc = _blendSrc;
            _defaultState->_blendDst = _blendDst;
        }
        if( (_bits & RS_CULL_FACE) && (_cullFaceEnabled != _defaultState->_cullFaceEnabled) )
        {
            if( _cullFaceEnabled )
                GL_ASSERT( glEnable(GL_CULL_FACE) );
            else
                GL_ASSERT( glDisable(GL_CULL_FACE) );
            _defaultState->_cullFaceEnabled = _cullFaceEnabled;
        }
        if( (_bits & RS_CULL_FACE_SIDE) && (_cullFaceSide != _defaultState->_cullFaceSide) )
        {
            GL_ASSERT( glCullFace(static_cast<GLenum>(_cullFaceSide)) );
            _defaultState->_cullFaceSide = _cullFaceSide;
        }
        if( (_bits & RS_FRONT_FACE) && (_frontFace != _defaultState->_frontFace) )
        {
            GL_ASSERT( glFrontFace(static_cast<GLenum>(_frontFace)) );
            _defaultState->_frontFace = _frontFace;
        }
        if( (_bits & RS_DEPTH_TEST) && (_depthTestEnabled != _defaultState->_depthTestEnabled) )
        {
            if( _depthTestEnabled )
                GL_ASSERT( glEnable(GL_DEPTH_TEST) );
            else
                GL_ASSERT( glDisable(GL_DEPTH_TEST) );
            _defaultState->_depthTestEnabled = _depthTestEnabled;
        }
        if( (_bits & RS_DEPTH_WRITE) && (_depthWriteEnabled != _defaultState->_depthWriteEnabled) )
        {
            GL_ASSERT( glDepthMask(_depthWriteEnabled ? GL_TRUE : GL_FALSE) );
            _defaultState->_depthWriteEnabled = _depthWriteEnabled;
        }
        if( (_bits & RS_DEPTH_FUNC) && (_depthFunction != _defaultState->_depthFunction) )
        {
            GL_ASSERT( glDepthFunc(static_cast<GLenum>(_depthFunction)) );
            _defaultState->_depthFunction = _depthFunction;
        }
        if( (_bits & RS_STENCIL_TEST) && (_stencilTestEnabled != _defaultState->_stencilTestEnabled) )
        {
            if( _stencilTestEnabled )
                GL_ASSERT( glEnable(GL_STENCIL_TEST) );
            else
                GL_ASSERT( glDisable(GL_STENCIL_TEST) );
            _defaultState->_stencilTestEnabled = _stencilTestEnabled;
        }
        if( (_bits & RS_STENCIL_WRITE) && (_stencilWrite != _defaultState->_stencilWrite) )
        {
            GL_ASSERT( glStencilMask(_stencilWrite) );
            _defaultState->_stencilWrite = _stencilWrite;
        }
        if( (_bits & RS_STENCIL_FUNC) && (_stencilFunction != _defaultState->_stencilFunction ||
            _stencilFunctionRef != _defaultState->_stencilFunctionRef ||
            _stencilFunctionMask != _defaultState->_stencilFunctionMask) )
        {
            GL_ASSERT( glStencilFunc(static_cast<GLenum>(_stencilFunction), _stencilFunctionRef, _stencilFunctionMask) );
            _defaultState->_stencilFunction = _stencilFunction;
            _defaultState->_stencilFunctionRef = _stencilFunctionRef;
            _defaultState->_stencilFunctionMask = _stencilFunctionMask;
        }
        if( (_bits & RS_STENCIL_OP) && (_stencilOpSfail != _defaultState->_stencilOpSfail ||
            _stencilOpDpfail != _defaultState->_stencilOpDpfail ||
            _stencilOpDppass != _defaultState->_stencilOpDppass) )
        {
            GL_ASSERT( glStencilOp(static_cast<GLenum>(_stencilOpSfail), static_cast<GLenum>(_stencilOpDpfail), static_cast<GLenum>(_stencilOpDppass)) );
            _defaultState->_stencilOpSfail = _stencilOpSfail;
            _defaultState->_stencilOpDpfail = _stencilOpDpfail;
            _defaultState->_stencilOpDppass = _stencilOpDppass;
        }

        _defaultState->_bits |= _bits;
    }


    void RenderState::StateBlock::restore(long stateOverrideBits)
    {
        BOOST_ASSERT(_defaultState);

        // If there is no state to restore (i.e. no non-default state), do nothing.
        if( _defaultState->_bits == 0 )
        {
            return;
        }

        // Restore any state that is not overridden and is not default
        if( !(stateOverrideBits & RS_BLEND) && (_defaultState->_bits & RS_BLEND) )
        {
            GL_ASSERT( glDisable(GL_BLEND) );
            _defaultState->_bits &= ~RS_BLEND;
            _defaultState->_blendEnabled = false;
        }
        if( !(stateOverrideBits & RS_BLEND_FUNC) && (_defaultState->_bits & RS_BLEND_FUNC) )
        {
            GL_ASSERT( glBlendFunc(GL_ONE, GL_ZERO) );
            _defaultState->_bits &= ~RS_BLEND_FUNC;
            _defaultState->_blendSrc = RenderState::BLEND_ONE;
            _defaultState->_blendDst = RenderState::BLEND_ZERO;
        }
        if( !(stateOverrideBits & RS_CULL_FACE) && (_defaultState->_bits & RS_CULL_FACE) )
        {
            GL_ASSERT( glDisable(GL_CULL_FACE) );
            _defaultState->_bits &= ~RS_CULL_FACE;
            _defaultState->_cullFaceEnabled = false;
        }
        if( !(stateOverrideBits & RS_CULL_FACE_SIDE) && (_defaultState->_bits & RS_CULL_FACE_SIDE) )
        {
            GL_ASSERT( glCullFace(GL_BACK) );
            _defaultState->_bits &= ~RS_CULL_FACE_SIDE;
            _defaultState->_cullFaceSide = RenderState::CULL_FACE_SIDE_BACK;
        }
        if( !(stateOverrideBits & RS_FRONT_FACE) && (_defaultState->_bits & RS_FRONT_FACE) )
        {
            GL_ASSERT( glFrontFace(GL_CCW) );
            _defaultState->_bits &= ~RS_FRONT_FACE;
            _defaultState->_frontFace = RenderState::FRONT_FACE_CCW;
        }
        if( !(stateOverrideBits & RS_DEPTH_TEST) && (_defaultState->_bits & RS_DEPTH_TEST) )
        {
            GL_ASSERT( glDisable(GL_DEPTH_TEST) );
            _defaultState->_bits &= ~RS_DEPTH_TEST;
            _defaultState->_depthTestEnabled = false;
        }
        if( !(stateOverrideBits & RS_DEPTH_WRITE) && (_defaultState->_bits & RS_DEPTH_WRITE) )
        {
            GL_ASSERT( glDepthMask(GL_TRUE) );
            _defaultState->_bits &= ~RS_DEPTH_WRITE;
            _defaultState->_depthWriteEnabled = true;
        }
        if( !(stateOverrideBits & RS_DEPTH_FUNC) && (_defaultState->_bits & RS_DEPTH_FUNC) )
        {
            GL_ASSERT( glDepthFunc(GL_LESS) );
            _defaultState->_bits &= ~RS_DEPTH_FUNC;
            _defaultState->_depthFunction = RenderState::DEPTH_LESS;
        }
        if( !(stateOverrideBits & RS_STENCIL_TEST) && (_defaultState->_bits & RS_STENCIL_TEST) )
        {
            GL_ASSERT( glDisable(GL_STENCIL_TEST) );
            _defaultState->_bits &= ~RS_STENCIL_TEST;
            _defaultState->_stencilTestEnabled = false;
        }
        if( !(stateOverrideBits & RS_STENCIL_WRITE) && (_defaultState->_bits & RS_STENCIL_WRITE) )
        {
            GL_ASSERT( glStencilMask(RS_ALL_ONES) );
            _defaultState->_bits &= ~RS_STENCIL_WRITE;
            _defaultState->_stencilWrite = RS_ALL_ONES;
        }
        if( !(stateOverrideBits & RS_STENCIL_FUNC) && (_defaultState->_bits & RS_STENCIL_FUNC) )
        {
            GL_ASSERT( glStencilFunc(static_cast<GLenum>(RenderState::STENCIL_ALWAYS), 0, RS_ALL_ONES) );
            _defaultState->_bits &= ~RS_STENCIL_FUNC;
            _defaultState->_stencilFunction = RenderState::STENCIL_ALWAYS;
            _defaultState->_stencilFunctionRef = 0;
            _defaultState->_stencilFunctionMask = RS_ALL_ONES;
        }
        if( !(stateOverrideBits & RS_STENCIL_OP) && (_defaultState->_bits & RS_STENCIL_OP) )
        {
            GL_ASSERT( glStencilOp(static_cast<GLenum>(RenderState::STENCIL_OP_KEEP), static_cast<GLenum>(RenderState::STENCIL_OP_KEEP), static_cast<GLenum>(RenderState::STENCIL_OP_KEEP)) );
            _defaultState->_bits &= ~RS_STENCIL_OP;
            _defaultState->_stencilOpSfail = RenderState::STENCIL_OP_KEEP;
            _defaultState->_stencilOpDpfail = RenderState::STENCIL_OP_KEEP;
            _defaultState->_stencilOpDppass = RenderState::STENCIL_OP_KEEP;
        }
    }


    void RenderState::StateBlock::enableDepthWrite()
    {
        BOOST_ASSERT(_defaultState);

        // Internal method used by Game::clear() to restore depth writing before a
        // clear operation. This is necessary if the last code to draw before the
        // next frame leaves depth writing disabled.
        if( !_defaultState->_depthWriteEnabled )
        {
            GL_ASSERT( glDepthMask(GL_TRUE) );
            _defaultState->_bits &= ~RS_DEPTH_WRITE;
            _defaultState->_depthWriteEnabled = true;
        }
    }


    void RenderState::StateBlock::setBlend(bool enabled)
    {
        _blendEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_BLEND;
        }
        else
        {
            _bits |= RS_BLEND;
        }
    }


    void RenderState::StateBlock::setBlendSrc(Blend blend)
    {
        _blendSrc = blend;
        if( _blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO )
        {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            _bits |= RS_BLEND_FUNC;
        }
    }


    void RenderState::StateBlock::setBlendDst(Blend blend)
    {
        _blendDst = blend;
        if( _blendSrc == BLEND_ONE && _blendDst == BLEND_ZERO )
        {
            // Default blend func
            _bits &= ~RS_BLEND_FUNC;
        }
        else
        {
            _bits |= RS_BLEND_FUNC;
        }
    }


    void RenderState::StateBlock::setCullFace(bool enabled)
    {
        _cullFaceEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_CULL_FACE;
        }
        else
        {
            _bits |= RS_CULL_FACE;
        }
    }


    void RenderState::StateBlock::setCullFaceSide(CullFaceSide side)
    {
        _cullFaceSide = side;
        if( _cullFaceSide == CULL_FACE_SIDE_BACK )
        {
            // Default cull side
            _bits &= ~RS_CULL_FACE_SIDE;
        }
        else
        {
            _bits |= RS_CULL_FACE_SIDE;
        }
    }


    void RenderState::StateBlock::setFrontFace(FrontFace winding)
    {
        _frontFace = winding;
        if( _frontFace == FRONT_FACE_CCW )
        {
            // Default front face
            _bits &= ~RS_FRONT_FACE;
        }
        else
        {
            _bits |= RS_FRONT_FACE;
        }
    }


    void RenderState::StateBlock::setDepthTest(bool enabled)
    {
        _depthTestEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_DEPTH_TEST;
        }
        else
        {
            _bits |= RS_DEPTH_TEST;
        }
    }


    void RenderState::StateBlock::setDepthWrite(bool enabled)
    {
        _depthWriteEnabled = enabled;
        if( enabled )
        {
            _bits &= ~RS_DEPTH_WRITE;
        }
        else
        {
            _bits |= RS_DEPTH_WRITE;
        }
    }


    void RenderState::StateBlock::setDepthFunction(DepthFunction func)
    {
        _depthFunction = func;
        if( _depthFunction == DEPTH_LESS )
        {
            // Default depth function
            _bits &= ~RS_DEPTH_FUNC;
        }
        else
        {
            _bits |= RS_DEPTH_FUNC;
        }
    }


    void RenderState::StateBlock::setStencilTest(bool enabled)
    {
        _stencilTestEnabled = enabled;
        if( !enabled )
        {
            _bits &= ~RS_STENCIL_TEST;
        }
        else
        {
            _bits |= RS_STENCIL_TEST;
        }
    }


    void RenderState::StateBlock::setStencilWrite(unsigned int mask)
    {
        _stencilWrite = mask;
        if( mask == RS_ALL_ONES )
        {
            // Default stencil write
            _bits &= ~RS_STENCIL_WRITE;
        }
        else
        {
            _bits |= RS_STENCIL_WRITE;
        }
    }


    void RenderState::StateBlock::setStencilFunction(StencilFunction func, int ref, unsigned int mask)
    {
        _stencilFunction = func;
        _stencilFunctionRef = ref;
        _stencilFunctionMask = mask;
        if( func == STENCIL_ALWAYS && ref == 0 && mask == RS_ALL_ONES )
        {
            // Default stencil function
            _bits &= ~RS_STENCIL_FUNC;
        }
        else
        {
            _bits |= RS_STENCIL_FUNC;
        }
    }


    void RenderState::StateBlock::setStencilOperation(StencilOperation sfail, StencilOperation dpfail, StencilOperation dppass)
    {
        _stencilOpSfail = sfail;
        _stencilOpDpfail = dpfail;
        _stencilOpDppass = dppass;
        if( sfail == STENCIL_OP_KEEP && dpfail == STENCIL_OP_KEEP && dppass == STENCIL_OP_KEEP )
        {
            // Default stencil operation
            _bits &= ~RS_STENCIL_OP;
        }
        else
        {
            _bits |= RS_STENCIL_OP;
        }
    }


    RenderState::AutoBindingResolver::AutoBindingResolver()
    {
        _customAutoBindingResolvers.push_back(this);
    }


    RenderState::AutoBindingResolver::~AutoBindingResolver()
    {
        auto itr = std::find(_customAutoBindingResolvers.begin(), _customAutoBindingResolvers.end(), this);
        if( itr != _customAutoBindingResolvers.end() )
            _customAutoBindingResolvers.erase(itr);
    }
}
