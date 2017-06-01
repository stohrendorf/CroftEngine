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
        : _shaderProgram{ShaderProgram::createFromFile(vshPath, fshPath, defines)}
    {
        if( _shaderProgram == nullptr )
        {
            BOOST_LOG_TRIVIAL(fatal) << "Failed to create shader. vertexShader = " << vshPath << ", fragmentShader = " << fshPath << ", defines = " << boost::algorithm::join(defines, "; ");
            BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader"));
        }
    }


    void Material::bind(const Node& node)
    {
        BOOST_ASSERT(_shaderProgram != nullptr);

        _shaderProgram->bind();

        RenderState::bind();

        for( const auto& param : _parameters )
        {
            BOOST_ASSERT(param);
            param->bind(node, _shaderProgram);
        }
    }


    std::shared_ptr<MaterialParameter> Material::getParameter(const std::string& name) const
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
}
