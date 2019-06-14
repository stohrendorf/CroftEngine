#pragma once

#include "glew.h"

namespace render
{
namespace gl
{
class Shader final
{
public:
    explicit Shader(const ::gl::GLenum type, const std::string& label = {})
            : m_handle{GL_ASSERT_FN( glCreateShader( type ) )}
            , m_type{type}
    {
        BOOST_ASSERT( type == ::gl::GL_VERTEX_SHADER || type == ::gl::GL_FRAGMENT_SHADER );
        BOOST_ASSERT( m_handle != 0 );

        if( !label.empty() )
        {
            GL_ASSERT( glObjectLabel( ::gl::GL_SHADER, m_handle, -1, label.c_str() ) );
        }
    }

    Shader(const Shader&) = delete;

    Shader(Shader&&) = delete;

    Shader& operator=(const Shader&) = delete;

    Shader& operator=(Shader&&) = delete;

    ~Shader()
    {
        GL_ASSERT( glDeleteShader( m_handle ) );
    }

    ::gl::GLenum getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const std::string& src)
    {
        const ::gl::GLchar* data[1]{src.c_str()};
        GL_ASSERT( glShaderSource( m_handle, 1, data, nullptr ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const ::gl::GLchar* src[], const ::gl::GLsizei n)
    {
        GL_ASSERT( glShaderSource( m_handle, n, src, nullptr ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void compile()
    {
        GL_ASSERT( glCompileShader( m_handle ) );
    }

    bool getCompileStatus() const
    {
        auto success = (::gl::GLint)::gl::GL_FALSE;
        GL_ASSERT( glGetShaderiv( m_handle, ::gl::GL_COMPILE_STATUS, &success ) );
        return success == (::gl::GLint)::gl::GL_TRUE;
    }

    std::string getInfoLog() const
    {
        ::gl::GLint length = 0;
        GL_ASSERT( glGetShaderiv( m_handle, ::gl::GL_INFO_LOG_LENGTH, &length ) );
        if( length == 0 )
        {
            length = 4096;
        }
        if( length > 0 )
        {
            const auto infoLog = new char[length];
            GL_ASSERT( glGetShaderInfoLog( m_handle, length, nullptr, infoLog ) );
            infoLog[length - 1] = '\0';
            std::string result = infoLog;
            delete[] infoLog;
            return result;
        }

        return {};
    }

    ::gl::GLuint getHandle() const noexcept
    {
        return m_handle;
    }

private:
    const ::gl::GLuint m_handle;

    const ::gl::GLenum m_type;
};
}
}
