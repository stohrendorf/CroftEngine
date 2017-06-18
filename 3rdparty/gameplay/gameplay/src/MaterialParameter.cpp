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
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(int value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(const float* values, size_t count)
    {
        std::vector<float> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const int* values, size_t count)
    {
        std::vector<int> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const glm::vec2& value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(const glm::vec2* values, size_t count)
    {
        std::vector<glm::vec2> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const glm::vec3& value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(const glm::vec3* values, size_t count)
    {
        std::vector<glm::vec3> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const glm::vec4& value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(const glm::vec4* values, size_t count)
    {
        std::vector<glm::vec4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const glm::mat4& value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(value);
        };
    }


    void MaterialParameter::set(const glm::mat4* values, size_t count)
    {
        std::vector<glm::mat4> tmp;
        tmp.assign(values, values + count);
        m_valueSetter = [tmp](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(tmp.data(), gsl::narrow<GLsizei>(tmp.size()));
        };
    }


    void MaterialParameter::set(const std::shared_ptr<gl::Texture>& texture)
    {
        m_valueSetter = [texture](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(*texture);
        };
    }


    void MaterialParameter::set(const std::vector<std::shared_ptr<gl::Texture>>& textures)
    {
        m_valueSetter = [textures](const Node& /*node*/, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(textures);
        };
    }


    gl::Program::ActiveUniform* MaterialParameter::getUniform(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        auto uniform = shaderProgram->getUniform(m_name);

        if( uniform )
            return uniform;

        if( (m_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 )
        {
            // This parameter was not found in the specified effect, so do nothing.
            BOOST_LOG_TRIVIAL(warning) << "Material parameter for uniform '" << m_name << "' not found in effect: '" << shaderProgram->getId() << "'.";
            m_loggerDirtyBits |= UNIFORM_NOT_FOUND;
        }

        return nullptr;
    }


    bool MaterialParameter::bind(const Node& node, const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        BOOST_ASSERT(shaderProgram);

        const auto mpsIt = node.getMaterialParameterSetters().find(m_name);
        if( !m_valueSetter && mpsIt == node.getMaterialParameterSetters().end() )
        {
            if( (m_loggerDirtyBits & PARAMETER_VALUE_NOT_SET) == 0 )
            {
                BOOST_LOG_TRIVIAL(warning) << "Material parameter value not set for: '" << m_name << "' in effect: '" << shaderProgram->getId() << "'.";
                m_loggerDirtyBits |= PARAMETER_VALUE_NOT_SET;
            }

            return false;
        }

        auto uniform = getUniform(shaderProgram);
        if( uniform == nullptr )
            return false;

        if( mpsIt != node.getMaterialParameterSetters().end() )
            mpsIt->second(node, *uniform);
        else
            (*m_valueSetter)(node, *uniform);

        return true;
    }


    void MaterialParameter::bindModelMatrix()
    {
        m_valueSetter = [](const Node& node, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(node.getModelMatrix());
        };
    }


    void MaterialParameter::bindViewMatrix()
    {
        m_valueSetter = [](const Node& node, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(node.getViewMatrix());
        };
    }


    void MaterialParameter::bindModelViewMatrix()
    {
        m_valueSetter = [](const Node& node, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(node.getViewMatrix() * node.getModelMatrix());
        };
    }


    void MaterialParameter::bindProjectionMatrix()
    {
        m_valueSetter = [](const Node& node, gl::Program::ActiveUniform& uniform)
        {
            uniform.set(node.getProjectionMatrix());
        };
    }
}
