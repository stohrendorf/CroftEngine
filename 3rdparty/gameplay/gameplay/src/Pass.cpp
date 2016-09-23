#include "Base.h"
#include "Pass.h"
#include "Technique.h"
#include "Material.h"
#include "Effect.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>

namespace gameplay
{
    Pass::Pass(const std::string& id, const std::shared_ptr<Technique>& technique)
        : _id(id)
        , _technique(technique)
        , _shaderProgram(nullptr)
    {
        RenderState::_parent = _technique;
    }


    Pass::~Pass() = default;


    bool Pass::initialize(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
    {
        // Attempt to create/load the effect.
        _shaderProgram = ShaderProgram::createFromFile(vshPath, fshPath, defines);
        if( _shaderProgram == nullptr )
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to create effect for pass. vertexShader = " << vshPath << ", fragmentShader = " << fshPath << ", defines = " << boost::algorithm::join(defines, "; ");
            return false;
        }

        return true;
    }


    const std::string& Pass::getId() const
    {
        return _id;
    }


    const std::shared_ptr<ShaderProgram>& Pass::getShaderProgram() const
    {
        return _shaderProgram;
    }


    void Pass::bind(const std::shared_ptr<VertexAttributeBinding>& vaBinding)
    {
        BOOST_ASSERT(_shaderProgram != nullptr);

        // Bind our effect.
        _shaderProgram->bind();

        // Bind our render state
        RenderState::bind(this);

        BOOST_ASSERT(_boundVaBinding == nullptr);
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
