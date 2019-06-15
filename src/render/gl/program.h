#pragma once

#include "texture.h"
#include "shader.h"
#include "glassert.h"

#include "gsl-lite.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace render
{
namespace gl
{
class Program : public BindableResource
{
public:
    explicit Program()
        : BindableResource{
        [](const ::gl::GLsizei n, ::gl::GLuint* handle) {
          BOOST_ASSERT( n == 1 && handle != nullptr );
          *handle = ::gl::glCreateProgram();
        },
        ::gl::glUseProgram,
        [](const ::gl::GLsizei n, const ::gl::GLuint* handle) {
          BOOST_ASSERT( n == 1 && handle != nullptr );
          ::gl::glDeleteProgram( *handle );
        }
    }
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attach(const Shader& shader)
    {
        BOOST_ASSERT( shader.getCompileStatus() );
        GL_ASSERT( glAttachShader( getHandle(), shader.getHandle() ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void link(const std::string& label = {})
    {
        GL_ASSERT( glLinkProgram( getHandle() ) );

        setLabel( ::gl::GL_PROGRAM, label );
    }

    bool getLinkStatus() const
    {
        ::gl::GLint success = (::gl::GLint)::gl::GL_FALSE;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_LINK_STATUS, &success ) );
        return success == (::gl::GLint)::gl::GL_TRUE;
    }

    std::string getInfoLog() const
    {
        ::gl::GLint length = 0;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_INFO_LOG_LENGTH, &length ) );
        if( length == 0 )
        {
            length = 4096;
        }
        if( length > 0 )
        {
            const auto infoLog = new char[length];
            GL_ASSERT( glGetProgramInfoLog( getHandle(), length, nullptr, infoLog ) );
            infoLog[length - 1] = '\0';
            std::string result = infoLog;
            delete[] infoLog;
            return result;
        }

        return {};
    }

    ::gl::GLint getActiveAttributeCount() const
    {
        ::gl::GLint activeAttributes = 0;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_ACTIVE_ATTRIBUTES, &activeAttributes ) );
        return activeAttributes;
    }

    ::gl::GLint getActiveAttributeMaxLength() const
    {
        ::gl::GLint length = 0;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length ) );
        return length;
    }

    ::gl::GLuint getActiveUniformCount() const
    {
        ::gl::GLint activeAttributes = 0;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_ACTIVE_UNIFORMS, &activeAttributes ) );
        return gsl::narrow<::gl::GLuint>( activeAttributes );
    }

    ::gl::GLint getActiveUniformMaxLength() const
    {
        ::gl::GLint length = 0;
        GL_ASSERT( glGetProgramiv( getHandle(), ::gl::GL_ACTIVE_UNIFORM_MAX_LENGTH, &length ) );
        return length;
    }

    class ActiveAttribute
    {
    public:
        explicit ActiveAttribute(const ::gl::GLuint program, const ::gl::GLuint index, const ::gl::GLint maxLength)
        {
            Expects( maxLength >= 0 );
            auto* attribName = new ::gl::GLchar[gsl::narrow_cast<size_t>( maxLength ) + 1];
            GL_ASSERT( glGetActiveAttrib( program, index, maxLength, nullptr, &m_size, &m_type, attribName ) );
            attribName[maxLength] = '\0';
            m_name = attribName;
            delete[] attribName;

            m_location = GL_ASSERT_FN( glGetAttribLocation( program, m_name.c_str() ) );
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        ::gl::GLint getLocation() const noexcept
        {
            return m_location;
        }

    private:
        ::gl::GLint m_size = 0;

        ::gl::GLenum m_type = ::gl::GL_NONE;

        std::string m_name{};

        ::gl::GLint m_location = -1;
    };

    class ActiveUniform
    {
    public:
        explicit ActiveUniform(const ::gl::GLuint program, const ::gl::GLuint index, const ::gl::GLint maxLength,
                               ::gl::GLint& samplerIndex)
            : m_program{ program }
        {
            auto* uniformName = new ::gl::GLchar[maxLength + 1];
            GL_ASSERT( glGetActiveUniform( program, index, maxLength, nullptr, &m_size, &m_type, uniformName ) );
            uniformName[maxLength] = '\0';

            m_name = uniformName;
            delete[] uniformName;

            m_location = GL_ASSERT_FN( glGetUniformLocation( program, m_name.c_str() ) );

            switch( m_type )
            {
            case ::gl::GL_SAMPLER_1D:
            case ::gl::GL_SAMPLER_1D_SHADOW:
            case ::gl::GL_SAMPLER_1D_ARRAY:
            case ::gl::GL_SAMPLER_1D_ARRAY_SHADOW:
            case ::gl::GL_SAMPLER_2D:
            case ::gl::GL_SAMPLER_2D_SHADOW:
            case ::gl::GL_SAMPLER_2D_ARRAY:
            case ::gl::GL_SAMPLER_2D_ARRAY_SHADOW:
            case ::gl::GL_SAMPLER_2D_RECT:
            case ::gl::GL_SAMPLER_2D_RECT_SHADOW:
            case ::gl::GL_SAMPLER_2D_MULTISAMPLE:
            case ::gl::GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
            case ::gl::GL_SAMPLER_3D:
            case ::gl::GL_SAMPLER_CUBE:
            case ::gl::GL_SAMPLER_CUBE_MAP_ARRAY:
            case ::gl::GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
            case ::gl::GL_SAMPLER_CUBE_SHADOW:
            case ::gl::GL_SAMPLER_BUFFER:
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
        void set(const ::gl::GLfloat value)
        {
            GL_ASSERT( glProgramUniform1f( m_program, m_location, value ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const ::gl::GLfloat* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniform1fv( m_program, m_location, count, values ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const ::gl::GLint value)
        {
            GL_ASSERT( glProgramUniform1i( m_program, m_location, value ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const ::gl::GLint* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniform1iv( m_program, m_location, count, values ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4& value)
        {
            GL_ASSERT( glProgramUniformMatrix4fv( m_program, m_location, 1, ::gl::GL_FALSE, value_ptr( value ) ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniformMatrix4fv( m_program, m_location, count, ::gl::GL_FALSE,
                                                  reinterpret_cast<const ::gl::GLfloat*>(values) ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2& value)
        {
            GL_ASSERT( glProgramUniform2f( m_program, m_location, value.x, value.y ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniform2fv( m_program, m_location, count,
                                            reinterpret_cast<const ::gl::GLfloat*>(values) ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3& value)
        {
            GL_ASSERT( glProgramUniform3f( m_program, m_location, value.x, value.y, value.z ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniform3fv( m_program, m_location, count,
                                            reinterpret_cast<const ::gl::GLfloat*>(values) ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<glm::vec3>& values)
        {
            set( values.data(), gsl::narrow<::gl::GLsizei>( values.size() ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4& value)
        {
            GL_ASSERT( glProgramUniform4f( m_program, m_location, value.x, value.y, value.z, value.w ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4* values, const ::gl::GLsizei count)
        {
            BOOST_ASSERT( values != nullptr );
            GL_ASSERT( glProgramUniform4fv( m_program, m_location, count,
                                            reinterpret_cast<const ::gl::GLfloat*>(values) ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const Texture& texture)
        {
            BOOST_ASSERT( m_samplerIndex >= 0 );

            GL_ASSERT( glActiveTexture( gsl::narrow<::gl::GLenum>( ::gl::GL_TEXTURE0 + m_samplerIndex ) ) );

            // Bind the sampler - this binds the texture and applies sampler state
            texture.bind();

            GL_ASSERT( glProgramUniform1i( m_program, m_location, m_samplerIndex ) );
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<std::shared_ptr<Texture>>& values)
        {
            BOOST_ASSERT( m_samplerIndex >= 0 );
            Expects( values.size() <= 32 );

            // Set samplers as active and load texture unit array
            std::vector<::gl::GLint> units;
            for( ::gl::GLint i = 0; i < values.size(); ++i )
            {
                GL_ASSERT( glActiveTexture( ::gl::GL_TEXTURE0 + m_samplerIndex + i ) );

                // Bind the sampler - this binds the texture and applies sampler state
                values[i]->bind();

                units.emplace_back( m_samplerIndex + i );
            }

            // Pass texture unit array to GL
            GL_ASSERT(
                glProgramUniform1iv( m_program, m_location, static_cast<::gl::GLsizei>(values.size()),
                                     units.data() ) );
        }

        ::gl::GLenum getType() const noexcept
        {
            return m_type;
        }

        ::gl::GLint getLocation() const noexcept
        {
            return m_location;
        }

        ::gl::GLint getSamplerIndex() const noexcept
        {
            return m_samplerIndex;
        }

    private:
        ::gl::GLint m_size = 0;

        ::gl::GLenum m_type = ::gl::GL_NONE;

        std::string m_name{};

        ::gl::GLint m_location = -1;

        ::gl::GLint m_samplerIndex = -1;

        const ::gl::GLuint m_program;
    };

    ActiveAttribute getActiveAttribute(const ::gl::GLuint index) const
    {
        return ActiveAttribute{ getHandle(), index, getActiveAttributeMaxLength() };
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

    ActiveUniform getActiveUniform(const ::gl::GLuint index, ::gl::GLint& samplerIndex) const
    {
        return ActiveUniform{ getHandle(), index, getActiveUniformMaxLength(), samplerIndex };
    }

    std::vector<ActiveUniform> getActiveUniforms() const
    {
        std::vector<ActiveUniform> uniforms;
        auto count = getActiveUniformCount();
        const auto maxLength = getActiveUniformMaxLength();
        ::gl::GLint samplerIndex = 0;
        for( decltype( count ) i = 0; i < count; ++i )
            uniforms.emplace_back( getHandle(), i, maxLength, samplerIndex );
        return uniforms;
    }
};
}
}
