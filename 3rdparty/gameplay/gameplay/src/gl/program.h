#pragma once

#include "texture.h"
#include "shader.h"

namespace gameplay
{
namespace gl
{
class Program : public BindableResource
{
public:
    explicit Program()
        : BindableResource{
            [](GLsizei n, GLuint* handle)
            {
                BOOST_ASSERT(n == 1 && handle != nullptr);
                *handle = glCreateProgram();
            },
            glUseProgram,
            [](GLsizei n, GLuint* handle)
            {
                BOOST_ASSERT(n == 1 && handle != nullptr);
                glDeleteProgram(*handle);
            }
        }
    {
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void attach(const gl::Shader& shader)
    {
        BOOST_ASSERT(shader.getCompileStatus());
        glAttachShader(getHandle(), shader.getHandle());
        checkGlError();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void link()
    {
        glLinkProgram(getHandle());
        checkGlError();
    }


    bool getLinkStatus() const
    {
        GLint success = GL_FALSE;
        glGetProgramiv(getHandle(), GL_LINK_STATUS, &success);
        checkGlError();
        return success == GL_TRUE;
    }


    std::string getInfoLog() const
    {
        GLint length = 0;
        glGetProgramiv(getHandle(), GL_INFO_LOG_LENGTH, &length);
        checkGlError();
        if( length == 0 )
        {
            length = 4096;
        }
        if( length > 0 )
        {
            char* infoLog = new char[length];
            glGetProgramInfoLog(getHandle(), length, nullptr, infoLog);
            checkGlError();
            infoLog[length - 1] = '\0';
            std::string result = infoLog;
            delete[] infoLog;
            return result;
        }

        return {};
    }


    GLint getActiveAttributeCount() const
    {
        GLint activeAttributes = 0;
        glGetProgramiv(getHandle(), GL_ACTIVE_ATTRIBUTES, &activeAttributes);
        checkGlError();
        return activeAttributes;
    }


    GLint getActiveAttributeMaxLength() const
    {
        GLint length = 0;
        glGetProgramiv(getHandle(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length);
        checkGlError();
        return length;
    }


    GLint getActiveUniformCount() const
    {
        GLint activeAttributes = 0;
        glGetProgramiv(getHandle(), GL_ACTIVE_UNIFORMS, &activeAttributes);
        checkGlError();
        return activeAttributes;
    }


    GLint getActiveUniformMaxLength() const
    {
        GLint length = 0;
        glGetProgramiv(getHandle(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &length);
        checkGlError();
        return length;
    }


    class ActiveAttribute
    {
    public:
        explicit ActiveAttribute(GLuint program, GLint index, GLint maxLength)
        {
            GLchar* attribName = new GLchar[maxLength + 1];
            glGetActiveAttrib(program, index, maxLength, nullptr, &m_size, &m_type, attribName);
            attribName[maxLength] = '\0';
            m_name = attribName;
            delete[] attribName;
            checkGlError();

            m_location = glGetAttribLocation(program, m_name.c_str());
            checkGlError();
        }


        const std::string& getName() const noexcept
        {
            return m_name;
        }


        GLint getLocation() const noexcept
        {
            return m_location;
        }


    private:
        GLint m_size = 0;
        GLenum m_type = 0;
        std::string m_name{};
        GLint m_location = -1;
    };


    class ActiveUniform
    {
    public:
        explicit ActiveUniform(GLuint program, GLint index, GLint maxLength, GLint& samplerIndex)
        {
            GLchar* uniformName = new GLchar[maxLength + 1];
            glGetActiveUniform(program, index, maxLength, nullptr, &m_size, &m_type, uniformName);
            uniformName[maxLength] = '\0';
            if( auto chr = std::strrchr(uniformName, '[') )
                *chr = '\0';

            m_name = uniformName;
            delete[] uniformName;
            checkGlError();

            m_location = glGetUniformLocation(program, m_name.c_str());
            checkGlError();

            switch( m_type )
            {
                case GL_SAMPLER_1D:
                case GL_SAMPLER_1D_SHADOW:
                case GL_SAMPLER_1D_ARRAY:
                case GL_SAMPLER_1D_ARRAY_SHADOW:
                case GL_SAMPLER_2D:
                case GL_SAMPLER_2D_SHADOW:
                case GL_SAMPLER_2D_ARRAY:
                case GL_SAMPLER_2D_ARRAY_SHADOW:
                case GL_SAMPLER_2D_RECT:
                case GL_SAMPLER_2D_RECT_SHADOW:
                case GL_SAMPLER_2D_MULTISAMPLE:
                case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
                case GL_SAMPLER_3D:
                case GL_SAMPLER_CUBE:
                case GL_SAMPLER_CUBE_MAP_ARRAY:
                case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
                case GL_SAMPLER_CUBE_SHADOW:
                case GL_SAMPLER_BUFFER:
                    m_samplerIndex = samplerIndex;
                    samplerIndex += m_size;
                default:
                    break;
            }
        }


        const std::string& getName() const noexcept
        {
            return m_name;
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(GLfloat value)
        {
            glUniform1f(m_location, value);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const GLfloat* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniform1fv(m_location, count, values);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(GLint value)
        {
            glUniform1i(m_location, value);
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const GLint* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniform1iv(m_location, count, values);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4& value)
        {
            glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::value_ptr(value));
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniformMatrix4fv(m_location, count, GL_FALSE, reinterpret_cast<const GLfloat*>(values));
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2& value)
        {
            glUniform2f(m_location, value.x, value.y);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniform2fv(m_location, count, reinterpret_cast<const GLfloat*>(values));
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3& value)
        {
            glUniform3f(m_location, value.x, value.y, value.z);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniform3fv(m_location, count, reinterpret_cast<const GLfloat*>(values));
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4& value)
        {
            glUniform4f(m_location, value.x, value.y, value.z, value.w);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4* values, GLsizei count)
        {
            BOOST_ASSERT(values != nullptr);
            glUniform4fv(m_location, count, reinterpret_cast<const GLfloat*>(values));
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const Texture& texture)
        {
            BOOST_ASSERT(m_samplerIndex >= 0);

            glActiveTexture(GL_TEXTURE0 + m_samplerIndex);
            checkGlError();

            // Bind the sampler - this binds the texture and applies sampler state
            texture.bind();

            glUniform1i(m_location, m_samplerIndex);
            checkGlError();
        }


        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<std::shared_ptr<Texture>>& values)
        {
            BOOST_ASSERT(m_samplerIndex >= 0);

            // Set samplers as active and load texture unit array
            GLint units[32];
            for( size_t i = 0; i < values.size(); ++i )
            {
                glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + m_samplerIndex + i));
                checkGlError();

                // Bind the sampler - this binds the texture and applies sampler state
                values[i]->bind();

                units[i] = static_cast<GLint>(m_samplerIndex + i);
            }

            // Pass texture unit array to GL
            glUniform1iv(m_location, static_cast<GLsizei>(values.size()), units);
            checkGlError();
        }


        GLenum getType() const noexcept
        {
            return m_type;
        }


    private:
        GLint m_size = 0;
        GLenum m_type = 0;
        std::string m_name{};
        GLint m_location = -1;
        GLint m_samplerIndex = -1;
    };


    ActiveAttribute getActiveAttribute(GLint index) const
    {
        return ActiveAttribute{getHandle(), index, getActiveAttributeMaxLength()};
    }


    std::vector<ActiveAttribute> getActiveAttributes() const
    {
        std::vector<ActiveAttribute> attribs;
        auto count = getActiveAttributeCount();
        const auto maxLength = getActiveAttributeMaxLength();
        for( decltype(count) i = 0; i < count; ++i )
            attribs.emplace_back(getHandle(), i, maxLength);
        return attribs;
    }


    ActiveUniform getActiveUniform(GLint index, GLint& samplerIndex) const
    {
        return ActiveUniform{getHandle(), index, getActiveUniformMaxLength(), samplerIndex};
    }


    std::vector<ActiveUniform> getActiveUniforms() const
    {
        std::vector<ActiveUniform> uniforms;
        auto count = getActiveUniformCount();
        const auto maxLength = getActiveUniformMaxLength();
        GLint samplerIndex = 0;
        for( decltype(count) i = 0; i < count; ++i )
            uniforms.emplace_back(getHandle(), i, maxLength, samplerIndex);
        return uniforms;
    }
};
}
}
