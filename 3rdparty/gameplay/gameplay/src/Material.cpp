#include "Base.h"
#include "Material.h"
#include "Effect.h"
#include "Technique.h"
#include "Pass.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>

namespace gameplay
{
    Material::Material() = default;


    Material::~Material() = default;


    std::shared_ptr<Material> Material::create(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        // Create a new material with a single technique and pass for the given effect.
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        material->_technique = std::make_shared<Technique>(nullptr, material);

        material->_technique->_pass = std::make_shared<Pass>(std::string(), material->_technique);
        material->_technique->_pass->_shaderProgram = shaderProgram;

        return material;
    }


    std::shared_ptr<Material> Material::create(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
    {
        // Create a new material with a single technique and pass for the given effect
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        material->_technique = std::make_shared<Technique>(nullptr, material);

        material->_technique->_pass = std::make_shared<Pass>(std::string(), material->_technique);
        if( !material->_technique->_pass->initialize(vshPath, fshPath, defines) )
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to create pass for material: vertexShader = " << vshPath << ", fragmentShader = " << fshPath << ", defines = " << boost::algorithm::join(defines, "; ");
            return nullptr;
        }

        return material;
    }


    void Material::setNodeBinding(Node* node)
    {
        RenderState::setNodeBinding(node);

        _technique->setNodeBinding(node);
    }
}
