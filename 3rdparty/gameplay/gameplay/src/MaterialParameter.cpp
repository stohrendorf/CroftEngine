#include "MaterialParameter.h"

#include "Node.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>


namespace gameplay
{
    MaterialParameter::MaterialParameter(const std::string& name)
        : m_name(name)
    {
    }


    MaterialParameter::~MaterialParameter() = default;


    const std::string& MaterialParameter::getName() const
    {
        return m_name;
    }


    void MaterialParameter::set(float value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(int value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const float* values, size_t count)
    {
        std::vector<float> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const int* values, size_t count)
    {
        std::vector<int> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const glm::vec2& value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const glm::vec2* values, size_t count)
    {
        std::vector<glm::vec2> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const glm::vec3& value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const glm::vec3* values, size_t count)
    {
        std::vector<glm::vec3> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const glm::vec4& value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const glm::vec4* values, size_t count)
    {
        std::vector<glm::vec4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const glm::mat4& value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const glm::mat4* values, size_t count)
    {
        std::vector<glm::mat4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, tmp.data(), tmp.size());
            };
    }


    void MaterialParameter::set(const std::shared_ptr<Texture::Sampler>& value)
    {
        m_valueSetter = [value](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
            {
                shaderProgram->setValue(*uniform, value);
            };
    }


    void MaterialParameter::set(const std::vector<std::shared_ptr<Texture::Sampler>>& samplers)
    {
        m_valueSetter = [samplers](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
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

        m_boundUniform = shaderProgram->getUniform(m_name);

        if( m_boundUniform )
            return true;

        if( (m_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 )
        {
            // This parameter was not found in the specified effect, so do nothing.
            BOOST_LOG_TRIVIAL(warning) << "Material parameter for uniform '" << m_name << "' not found in effect: '" << shaderProgram->getId() << "'.";
            m_loggerDirtyBits |= UNIFORM_NOT_FOUND;
        }

        return false;
    }


    void MaterialParameter::bind(const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        if( !m_valueSetter )
        {
            if( (m_loggerDirtyBits & PARAMETER_VALUE_NOT_SET) == 0 )
            {
                BOOST_LOG_TRIVIAL(warning) << "Material parameter value not set for: '" << m_name << "' in effect: '" << shaderProgram->getId() << "'.";
                m_loggerDirtyBits |= PARAMETER_VALUE_NOT_SET;
            }

            return;
        }

        if( !updateUniformBinding(shaderProgram) )
            return;

        (*m_valueSetter)(node, shaderProgram, m_boundUniform);
    }

    void MaterialParameter::bindWorldViewProjectionMatrix()
    {
        m_valueSetter = [](const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram, const std::shared_ptr<Uniform>& uniform)
        {
            shaderProgram->setValue(*uniform, node.getWorldViewProjectionMatrix());
        };
    }
}
