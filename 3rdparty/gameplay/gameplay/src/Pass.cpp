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
        , _shaderProgram(nullptr)
    {
        RenderState::_parent = _technique;
    }


    Pass::~Pass() = default;


    bool Pass::initialize(const char* vshPath, const char* fshPath, const char* defines)
    {
        GP_ASSERT(vshPath);
        GP_ASSERT(fshPath);

        // Attempt to create/load the effect.
        _shaderProgram = ShaderProgram::createFromFile(vshPath, fshPath, defines);
        if( _shaderProgram == nullptr )
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


    const std::shared_ptr<ShaderProgram>& Pass::getShaderProgram() const
    {
        return _shaderProgram;
    }


    void Pass::bind(const std::shared_ptr<VertexAttributeBinding>& vaBinding)
    {
        GP_ASSERT(_shaderProgram != nullptr);

        // Bind our effect.
        _shaderProgram->bind();

        // Bind our render state
        RenderState::bind(this);

        GP_ASSERT(_boundVaBinding == nullptr);
        _boundVaBinding = vaBinding;

        // If we have a vertex attribute binding, bind it
        if( vaBinding )
        {
            vaBinding->bind();
        }
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Pass::unbind()
    {
        // If we have a vertex attribute binding, unbind it
        if( _boundVaBinding )
        {
            _boundVaBinding->unbind();
            _boundVaBinding.reset();
        }
    }
}
