#pragma once

#include "texture.h"
#include "shader.h"

#include "gsl-lite.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace gameplay
{
namespace gl
{
class Program : public BindableResource
{
public:
    explicit Program()
            : BindableResource{
            [](const GLsizei n, GLuint* handle) {
                BOOST_ASSERT( n == 1 && handle != nullptr );
                *handle = glCreateProgram();
            },
            glUseProgram,
            [](const GLsizei n, const GLuint* handle) {
                BOOST_ASSERT( n == 1 && handle != nullptr );
                glDeleteProgram( *handle );
            }
    }
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attach(const Shader& shader)
    {
        BOOST_ASSERT( shader.getCompileStatus() );
        glAttachShader( getHandle(), shader.getHandle() );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void link(const std::string& label = {})
    {
        glLinkProgram( getHandle() );
        checkGlError();

        setLabel( GL_PROGRAM, label );
    }

    bool getLinkStatus() const
    {
        GLint success = GL_FALSE;
        glGetProgramiv( getHandle(), GL_LINK_STATUS, &success );
        checkGlError();
        return success == GL_TRUE;
    }

    std::string getInfoLog() const
    {
        GLint length = 0;
        glGetProgramiv( getHandle(), GL_INFO_LOG_LENGTH, &length );
        checkGlError();
        if( length == 0 )
        {
            length = 4096;
        }
        if( length > 0 )
        {
            const auto infoLog = new char[length];
            glGetProgramInfoLog( getHandle(), length, nullptr, infoLog );
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
        glGetProgramiv( getHandle(), GL_ACTIVE_ATTRIBUTES, &activeAttributes );
        checkGlError();
        return activeAttributes;
    }

    GLint getActiveAttributeMaxLength() const
    {
        GLint length = 0;
        glGetProgramiv( getHandle(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length );
        checkGlError();
        return length;
    }

    GLuint getActiveUniformCount() const
    {
        GLint activeAttributes = 0;
        glGetProgramiv( getHandle(), GL_ACTIVE_UNIFORMS, &activeAttributes );
        checkGlError();
        return gsl::narrow<GLuint>( activeAttributes );
    }

    GLint getActiveUniformMaxLength() const
    {
        GLint length = 0;
        glGetProgramiv( getHandle(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &length );
        checkGlError();
        return length;
    }


    class ActiveAttribute
    {
    public:
        explicit ActiveAttribute(const GLuint program, const GLuint index, const GLint maxLength)
        {
            Expects( maxLength >= 0 );
            auto* attribName = new GLchar[gsl::narrow_cast<size_t>( maxLength ) + 1];
            glGetActiveAttrib( program, index, maxLength, nullptr, &m_size, &m_type, attribName );
            attribName[maxLength] = '\0';
            m_name = attribName;
            delete[] attribName;
            checkGlError();

            m_location = glGetAttribLocation( program, m_name.c_str() );
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
        explicit ActiveUniform(const GLuint program, const GLuint index, const GLint maxLength, GLint& samplerIndex)
                : m_program{program}
        {
            auto* uniformName = new GLchar[maxLength + 1];
            glGetActiveUniform( program, index, maxLength, nullptr, &m_size, &m_type, uniformName );
            uniformName[maxLength] = '\0';
            if( const auto chr = strrchr( uniformName, '[' ) )
                *chr = '\0';

            m_name = uniformName;
            delete[] uniformName;
            checkGlError();

            m_location = glGetUniformLocation( program, m_name.c_str() );
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
        void set(const GLfloat value)
        {
            glProgramUniform1f( m_program, m_location, value );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const GLfloat* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniform1fv( m_program, m_location, count, values );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const GLint value)
        {
            glProgramUniform1i( m_program, m_location, value );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const GLint* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniform1iv( m_program, m_location, count, values );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4& value)
        {
            glProgramUniformMatrix4fv( m_program, m_location, 1, GL_FALSE, value_ptr( value ) );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniformMatrix4fv( m_program, m_location, count, GL_FALSE,
                                       reinterpret_cast<const GLfloat*>(values) );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2& value)
        {
            glProgramUniform2f( m_program, m_location, value.x, value.y );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniform2fv( m_program, m_location, count, reinterpret_cast<const GLfloat*>(values) );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3& value)
        {
            glProgramUniform3f( m_program, m_location, value.x, value.y, value.z );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniform3fv( m_program, m_location, count, reinterpret_cast<const GLfloat*>(values) );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4& value)
        {
            glProgramUniform4f( m_program, m_location, value.x, value.y, value.z, value.w );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4* values, const GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            glProgramUniform4fv( m_program, m_location, count, reinterpret_cast<const GLfloat*>(values) );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const Texture& texture)
        {
            BOOST_ASSERT( m_samplerIndex >= 0 );

            glActiveTexture( gsl::narrow<GLenum>( GL_TEXTURE0 + m_samplerIndex ) );
            checkGlError();

            // Bind the sampler - this binds the texture and applies sampler state
            texture.bind();

            glProgramUniform1i( m_program, m_location, m_samplerIndex );
            checkGlError();
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<std::shared_ptr<Texture>>& values)
        {
            BOOST_ASSERT( m_samplerIndex >= 0 );

            // Set samplers as active and load texture unit array
            std::vector<GLint> units;
            for( std::size_t i = 0; i < values.size(); ++i )
            {
                glActiveTexture( static_cast<GLenum>(GL_TEXTURE0 + m_samplerIndex + i) );
                checkGlError();

                // Bind the sampler - this binds the texture and applies sampler state
                values[i]->bind();

                units.emplace_back( static_cast<GLint>(m_samplerIndex + i) );
            }

            // Pass texture unit array to GL
            glProgramUniform1iv( m_program, m_location, static_cast<GLsizei>(values.size()), units.data() );
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

        const GLuint m_program;
    };


    ActiveAttribute getActiveAttribute(const GLuint index) const
    {
        return ActiveAttribute{getHandle(), index, getActiveAttributeMaxLength()};
    }

    std::vector<ActiveAttribute> getActiveAttributes() const
    {
        std::vector<ActiveAttribute> attribs;
        auto count = getActiveAttributeCount();
        const auto maxLength = getActiveAttributeMaxLength();
        for( decltype( count ) i = 0; i < count; ++i )
            attribs.emplace_back( getHandle(), i, maxLength );
        return attribs;
    }

    ActiveUniform getActiveUniform(const GLuint index, GLint& samplerIndex) const
    {
        return ActiveUniform{getHandle(), index, getActiveUniformMaxLength(), samplerIndex};
    }

    std::vector<ActiveUniform> getActiveUniforms() const
    {
        std::vector<ActiveUniform> uniforms;
        auto count = getActiveUniformCount();
        const auto maxLength = getActiveUniformMaxLength();
        GLint samplerIndex = 0;
        for( decltype( count ) i = 0; i < count; ++i )
            uniforms.emplace_back( getHandle(), i, maxLength, samplerIndex );
        return uniforms;
    }
};
}
}
