#include "Base.h"
#include "Pass.h"
#include "Technique.h"
#include "Material.h"
#include "Effect.h"


namespace gameplay
{
    Pass::Pass(const char* id, const std::shared_ptr<Technique>& technique)
        : _id(id ? id : "")
        , _technique(technique)
        , _effect(nullptr)
        , _vaBinding(nullptr)
    {
        RenderState::_parent = _technique;
    }


    Pass::~Pass() = default;


    bool Pass::initialize(const char* vshPath, const char* fshPath, const char* defines)
    {
        GP_ASSERT(vshPath);
        GP_ASSERT(fshPath);

        _vaBinding.reset();

        // Attempt to create/load the effect.
        _effect = Effect::createFromFile(vshPath, fshPath, defines);
        if( _effect == nullptr )
        {
            GP_WARN("Failed to create effect for pass. vertexShader = %s, fragmentShader = %s, defines = %s", vshPath, fshPath, defines ? defines : "");
            return false;
        }

        return true;
    }


    const char* Pass::getId() const
    {
        return _id.c_str();
    }


    const std::shared_ptr<Effect>& Pass::getEffect() const
    {
        return _effect;
    }


    void Pass::setVertexAttributeBinding(const std::shared_ptr<VertexAttributeBinding>& binding)
    {
        _vaBinding = binding;
    }


    const std::shared_ptr<VertexAttributeBinding>& Pass::getVertexAttributeBinding() const
    {
        return _vaBinding;
    }


    void Pass::bind()
    {
        GP_ASSERT(_effect);

        // Bind our effect.
        _effect->bind();

        // Bind our render state
        RenderState::bind(this);

        // If we have a vertex attribute binding, bind it
        if( _vaBinding )
        {
            _vaBinding->bind();
        }
    }


    void Pass::unbind()
    {
        // If we have a vertex attribute binding, unbind it
        if( _vaBinding )
        {
            _vaBinding->unbind();
        }
    }
}
