#include "Base.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>

namespace gameplay
{
    Material::Material(const std::shared_ptr<ShaderProgram>& shaderProgram)
        : _shaderProgram{shaderProgram}
    {
        BOOST_ASSERT(shaderProgram != nullptr);
    }


    Material::~Material() = default;


    Material::Material(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
        : _shaderProgram{ ShaderProgram::createFromFile(vshPath, fshPath, defines) }
    {
        if(_shaderProgram == nullptr)
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create shader. vertexShader = " << vshPath << ", fragmentShader = " << fshPath << ", defines = " << boost::algorithm::join(defines, "; ");
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader"));
        }
    }


    void Material::bind(const std::shared_ptr<VertexAttributeBinding>& vaBinding)
    {
        BOOST_ASSERT(_shaderProgram != nullptr);

        // Bind our effect.
        _shaderProgram->bind();

        // Bind our render state
        RenderState::bind(this);

        BOOST_ASSERT(_boundVaBinding == nullptr);
        _boundVaBinding = vaBinding;

        // If we have a vertex attribute binding, bind it
        if(vaBinding)
        {
            vaBinding->bind();
        }
    }
}
