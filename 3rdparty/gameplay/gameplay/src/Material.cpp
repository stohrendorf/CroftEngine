#include "Base.h"
#include "Material.h"
#include "Effect.h"
#include "Technique.h"
#include "Pass.h"
#include "Node.h"
#include "MaterialParameter.h"


namespace gameplay
{
    Material::Material() = default;


    Material::~Material() = default;


    std::shared_ptr<Material> Material::create(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        GP_ASSERT(shaderProgram);

        // Create a new material with a single technique and pass for the given effect.
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        material->_technique = std::make_shared<Technique>(nullptr, material);

        material->_technique->_pass = std::make_shared<Pass>(nullptr, material->_technique);
        material->_technique->_pass->_shaderProgram = shaderProgram;

        return material;
    }


    std::shared_ptr<Material> Material::create(const char* vshPath, const char* fshPath, const char* defines)
    {
        GP_ASSERT(vshPath);
        GP_ASSERT(fshPath);

        // Create a new material with a single technique and pass for the given effect
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        material->_technique = std::make_shared<Technique>(nullptr, material);

        material->_technique->_pass = std::make_shared<Pass>(nullptr, material->_technique);
        if( !material->_technique->_pass->initialize(vshPath, fshPath, defines) )
        {
            GP_WARN("Failed to create pass for material: vertexShader = %s, fragmentShader = %s, defines = %s", vshPath, fshPath, defines ? defines : "");
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
