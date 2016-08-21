#include "Base.h"
#include "Material.h"
#include "FileSystem.h"
#include "Effect.h"
#include "Technique.h"
#include "Pass.h"
#include "Properties.h"
#include "Node.h"


namespace gameplay
{
    Material::Material() :
                         _currentTechnique(nullptr)
    {
    }


    Material::~Material()
    {
        // Destroy all the techniques.
        for( size_t i = 0, count = _techniques.size(); i < count; ++i )
        {
            Technique* technique = _techniques[i];
            SAFE_RELEASE(technique);
        }
    }


    Material* Material::create(Effect* effect)
    {
        GP_ASSERT(effect);

        // Create a new material with a single technique and pass for the given effect.
        Material* material = new Material();

        Technique* technique = new Technique(nullptr, material);
        material->_techniques.push_back(technique);

        Pass* pass = new Pass(nullptr, technique);
        pass->_effect = effect;
        technique->_passes.push_back(pass);
        effect->addRef();

        material->_currentTechnique = technique;

        return material;
    }


    Material* Material::create(const char* vshPath, const char* fshPath, const char* defines)
    {
        GP_ASSERT(vshPath);
        GP_ASSERT(fshPath);

        // Create a new material with a single technique and pass for the given effect
        Material* material = new Material();

        Technique* technique = new Technique(nullptr, material);
        material->_techniques.push_back(technique);

        Pass* pass = new Pass(nullptr, technique);
        if( !pass->initialize(vshPath, fshPath, defines) )
        {
            GP_WARN("Failed to create pass for material: vertexShader = %s, fragmentShader = %s, defines = %s", vshPath, fshPath, defines ? defines : "");
            SAFE_RELEASE(pass);
            SAFE_RELEASE(material);
            return nullptr;
        }
        technique->_passes.push_back(pass);

        material->_currentTechnique = technique;

        return material;
    }


    unsigned int Material::getTechniqueCount() const
    {
        return (unsigned int)_techniques.size();
    }


    Technique* Material::getTechniqueByIndex(unsigned int index) const
    {
        GP_ASSERT(index < _techniques.size());
        return _techniques[index];
    }


    Technique* Material::getTechnique(const char* id) const
    {
        GP_ASSERT(id);
        for( size_t i = 0, count = _techniques.size(); i < count; ++i )
        {
            Technique* t = _techniques[i];
            GP_ASSERT(t);
            if( strcmp(t->getId(), id) == 0 )
            {
                return t;
            }
        }

        return nullptr;
    }


    Technique* Material::getTechnique() const
    {
        return _currentTechnique;
    }


    void Material::setTechnique(const char* id)
    {
        Technique* t = getTechnique(id);
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


    Material* Material::clone(NodeCloneContext& context) const
    {
        Material* material = new Material();
        RenderState::cloneInto(material, context);

        for( std::vector<Technique*>::const_iterator it = _techniques.begin(); it != _techniques.end(); ++it )
        {
            const Technique* technique = *it;
            GP_ASSERT(technique);
            Technique* techniqueClone = technique->clone(material, context);
            material->_techniques.push_back(techniqueClone);
            if( _currentTechnique == technique )
            {
                material->_currentTechnique = techniqueClone;
            }
        }
        return material;
    }


    bool Material::loadTechnique(Material* material, Properties* techniqueProperties, PassCallback callback, void* cookie)
    {
        GP_ASSERT(material);
        GP_ASSERT(techniqueProperties);

        // Create a new technique.
        Technique* technique = new Technique(techniqueProperties->getId(), material);

        // Load uniform value parameters for this technique.
        loadRenderState(technique, techniqueProperties);

        // Go through all the properties and create passes under this technique.
        techniqueProperties->rewind();
        Properties* passProperties = nullptr;
        while( (passProperties = techniqueProperties->getNextNamespace()) )
        {
            if( strcmp(passProperties->getNamespace(), "pass") == 0 )
            {
                // Create and load passes.
                if( !loadPass(technique, passProperties, callback, cookie) )
                {
                    GP_ERROR("Failed to create pass for technique.");
                    SAFE_RELEASE(technique);
                    return false;
                }
            }
        }

        // Add the new technique to the material.
        material->_techniques.push_back(technique);

        return true;
    }


    bool Material::loadPass(Technique* technique, Properties* passProperties, PassCallback callback, void* cookie)
    {
        GP_ASSERT(passProperties);
        GP_ASSERT(technique);

        // Fetch shader info required to create the effect of this technique.
        const char* vertexShaderPath = passProperties->getString("vertexShader");
        GP_ASSERT(vertexShaderPath);
        const char* fragmentShaderPath = passProperties->getString("fragmentShader");
        GP_ASSERT(fragmentShaderPath);
        const char* passDefines = passProperties->getString("defines");

        // Create the pass
        Pass* pass = new Pass(passProperties->getId(), technique);

        // Load render state.
        loadRenderState(pass, passProperties);

        // If a pass callback was specified, call it and add the result to our list of defines
        std::string allDefines = passDefines ? passDefines : "";
        if( callback )
        {
            std::string customDefines = callback(pass, cookie);
            if( customDefines.length() > 0 )
            {
                if( allDefines.length() > 0 )
                    allDefines += ';';
                allDefines += customDefines;
            }
        }

        // Initialize/compile the effect with the full set of defines
        if( !pass->initialize(vertexShaderPath, fragmentShaderPath, allDefines.c_str()) )
        {
            GP_WARN("Failed to create pass for technique.");
            SAFE_RELEASE(pass);
            return false;
        }

        // Add the new pass to the technique.
        technique->_passes.push_back(pass);

        return true;
    }


    static bool isMaterialKeyword(const char* str)
    {
        GP_ASSERT(str);

#define MATERIAL_KEYWORD_COUNT 3
        static const char* reservedKeywords[MATERIAL_KEYWORD_COUNT] =
            {
                "vertexShader",
                "fragmentShader",
                "defines"
            };
        for( unsigned int i = 0; i < MATERIAL_KEYWORD_COUNT; ++i )
        {
            if( strcmp(reservedKeywords[i], str) == 0 )
            {
                return true;
            }
        }
        return false;
    }


    static Texture::Filter parseTextureFilterMode(const char* str, Texture::Filter defaultValue)
    {
        if( str == nullptr || strlen(str) == 0 )
        {
            GP_ERROR("Texture filter mode string must be non-nullptr and non-empty.");
            return defaultValue;
        }
        else if( strcmp(str, "NEAREST") == 0 )
        {
            return Texture::NEAREST;
        }
        else if( strcmp(str, "LINEAR") == 0 )
        {
            return Texture::LINEAR;
        }
        else if( strcmp(str, "NEAREST_MIPMAP_NEAREST") == 0 )
        {
            return Texture::NEAREST_MIPMAP_NEAREST;
        }
        else if( strcmp(str, "LINEAR_MIPMAP_NEAREST") == 0 )
        {
            return Texture::LINEAR_MIPMAP_NEAREST;
        }
        else if( strcmp(str, "NEAREST_MIPMAP_LINEAR") == 0 )
        {
            return Texture::NEAREST_MIPMAP_LINEAR;
        }
        else if( strcmp(str, "LINEAR_MIPMAP_LINEAR") == 0 )
        {
            return Texture::LINEAR_MIPMAP_LINEAR;
        }
        else
        {
            GP_ERROR("Unsupported texture filter mode string ('%s').", str);
            return defaultValue;
        }
    }


    static Texture::Wrap parseTextureWrapMode(const char* str, Texture::Wrap defaultValue)
    {
        if( str == nullptr || strlen(str) == 0 )
        {
            GP_ERROR("Texture wrap mode string must be non-nullptr and non-empty.");
            return defaultValue;
        }
        else if( strcmp(str, "REPEAT") == 0 )
        {
            return Texture::REPEAT;
        }
        else if( strcmp(str, "CLAMP") == 0 )
        {
            return Texture::CLAMP;
        }
        else
        {
            GP_ERROR("Unsupported texture wrap mode string ('%s').", str);
            return defaultValue;
        }
    }


    void Material::loadRenderState(RenderState* renderState, Properties* properties)
    {
        GP_ASSERT(renderState);
        GP_ASSERT(properties);

        // Rewind the properties to start reading from the start.
        properties->rewind();

        const char* name;
        while( (name = properties->getNextProperty()) )
        {
            if( isMaterialKeyword(name) )
                continue; // keyword - skip

            switch( properties->getType() )
            {
                case Properties::NUMBER:
                    GP_ASSERT(renderState->getParameter(name));
                    renderState->getParameter(name)->setValue(properties->getFloat());
                    break;
                case Properties::VECTOR2:
                {
                    Vector2 vector2;
                    if( properties->getVector2(nullptr, &vector2) )
                    {
                        GP_ASSERT(renderState->getParameter(name));
                        renderState->getParameter(name)->setValue(vector2);
                    }
                }
                    break;
                case Properties::VECTOR3:
                {
                    Vector3 vector3;
                    if( properties->getVector3(nullptr, &vector3) )
                    {
                        GP_ASSERT(renderState->getParameter(name));
                        renderState->getParameter(name)->setValue(vector3);
                    }
                }
                    break;
                case Properties::VECTOR4:
                {
                    Vector4 vector4;
                    if( properties->getVector4(nullptr, &vector4) )
                    {
                        GP_ASSERT(renderState->getParameter(name));
                        renderState->getParameter(name)->setValue(vector4);
                    }
                }
                    break;
                case Properties::MATRIX:
                {
                    Matrix matrix;
                    if( properties->getMatrix(nullptr, &matrix) )
                    {
                        GP_ASSERT(renderState->getParameter(name));
                        renderState->getParameter(name)->setValue(matrix);
                    }
                }
                    break;
                default:
                {
                    // Assume this is a parameter auto-binding.
                    renderState->setParameterAutoBinding(name, properties->getString());
                }
                    break;
            }
        }

        // Iterate through all child namespaces searching for samplers and render state blocks.
        Properties* ns;
        while( (ns = properties->getNextNamespace()) )
        {
            if( strcmp(ns->getNamespace(), "renderState") == 0 )
            {
                while( (name = ns->getNextProperty()) )
                {
                    GP_ASSERT(renderState->getStateBlock());
                    renderState->getStateBlock()->setState(name, ns->getString());
                }
            }
        }
    }
}
