#include "Base.h"
#include "Material.h"
#include "FileSystem.h"
#include "Effect.h"
#include "Technique.h"
#include "Pass.h"
#include "Node.h"
#include "MaterialParameter.h"


namespace gameplay
{
    Material::Material() = default;


    Material::~Material() = default;


    std::shared_ptr<Material> Material::create(const std::shared_ptr<Effect>& effect)
    {
        GP_ASSERT(effect);

        // Create a new material with a single technique and pass for the given effect.
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        auto technique = std::make_shared<Technique>(nullptr, material);
        material->_techniques.emplace_back(technique);

        Pass* pass = new Pass(nullptr, technique);
        pass->_effect = effect;
        technique->_passes.emplace_back(pass);

        material->_currentTechnique = technique;

        return material;
    }


    std::shared_ptr<Material> Material::create(const char* vshPath, const char* fshPath, const char* defines)
    {
        GP_ASSERT(vshPath);
        GP_ASSERT(fshPath);

        // Create a new material with a single technique and pass for the given effect
        std::shared_ptr<Material> material{ std::make_shared<Material>() };

        auto technique = std::make_shared<Technique>(nullptr, material);
        material->_techniques.push_back(technique);

        auto pass = std::make_shared<Pass>(nullptr, technique);
        if( !pass->initialize(vshPath, fshPath, defines) )
        {
            GP_WARN("Failed to create pass for material: vertexShader = %s, fragmentShader = %s, defines = %s", vshPath, fshPath, defines ? defines : "");
            return nullptr;
        }
        technique->_passes.emplace_back(pass);

        material->_currentTechnique = technique;

        return material;
    }


    size_t Material::getTechniqueCount() const
    {
        return _techniques.size();
    }


    const std::shared_ptr<Technique>& Material::getTechniqueByIndex(size_t index) const
    {
        GP_ASSERT(index < _techniques.size());
        return _techniques[index];
    }


    std::shared_ptr<Technique> Material::getTechnique(const char* id) const
    {
        GP_ASSERT(id);
        for( size_t i = 0, count = _techniques.size(); i < count; ++i )
        {
            auto t = _techniques[i];
            GP_ASSERT(t);
            if( strcmp(t->getId(), id) == 0 )
            {
                return t;
            }
        }

        return nullptr;
    }


    const std::shared_ptr<Technique>& Material::getTechnique() const
    {
        return _currentTechnique;
    }


    void Material::setTechnique(const char* id)
    {
        auto t = getTechnique(id);
        if( t )
        {
            _currentTechnique = t;
        }
    }


    void Material::setNodeBinding(Node* node)
    {
        RenderState::setNodeBinding(node);

        for( size_t i = 0, count = _techniques.size(); i < count; ++i )
        {
            _techniques[i]->setNodeBinding(node);
        }
    }
}
