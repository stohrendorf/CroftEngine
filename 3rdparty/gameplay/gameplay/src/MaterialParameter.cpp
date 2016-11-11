#include "Base.h"
#include "MaterialParameter.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>


namespace gameplay
{
    MaterialParameter::MaterialParameter(const std::string& name)
        : _name(name)
    {
    }


    MaterialParameter::~MaterialParameter() = default;


    const std::string& MaterialParameter::getName() const
    {
        return _name;
    }


    void MaterialParameter::setValue(float value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(int value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const float* values, size_t count)
    {
        std::vector<float> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const int* values, size_t count)
    {
        std::vector<int> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const glm::vec2& value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const glm::vec2* values, size_t count)
    {
        std::vector<glm::vec2> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const glm::vec3& value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const glm::vec3* values, size_t count)
    {
        std::vector<glm::vec3> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const glm::vec4& value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const glm::vec4* values, size_t count)
    {
        std::vector<glm::vec4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const glm::mat4& value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const glm::mat4* values, size_t count)
    {
        std::vector<glm::mat4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::setValue(const std::shared_ptr<Texture::Sampler>& value)
    {
        m_valueSetter = [value](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::setValue(const std::vector<std::shared_ptr<Texture::Sampler>>& samplers)
    {
        m_valueSetter = [samplers](const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, samplers);
            };
    }


    bool MaterialParameter::updateUniformBinding(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        // If we had a Uniform cached that is not from the passed in effect,
        // we need to update our uniform to point to the new effect's uniform.
        if( m_boundUniform && m_boundUniform->getShaderProgram() == shaderProgram )
            return true;

        m_boundUniform = shaderProgram->getUniform(_name);

        if( m_boundUniform )
            return true;

        if( (_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 )
        {
            // This parameter was not found in the specified effect, so do nothing.
            BOOST_LOG_TRIVIAL(warning) << "Material parameter for uniform '" << _name << "' not found in effect: '" << shaderProgram->getId() << "'.";
            _loggerDirtyBits |= UNIFORM_NOT_FOUND;
        }

        return false;
    }


    void MaterialParameter::bind(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        if( !m_valueSetter )
        {
            if( (_loggerDirtyBits & PARAMETER_VALUE_NOT_SET) == 0 )
            {
                BOOST_LOG_TRIVIAL(warning) << "Material parameter value not set for: '" << _name << "' in effect: '" << shaderProgram->getId() << "'.";
                _loggerDirtyBits |= PARAMETER_VALUE_NOT_SET;
            }

            return;
        }

        if( !updateUniformBinding(shaderProgram) )
            return;

        (*m_valueSetter)(shaderProgram, m_boundUniform);
    }
}
