#pragma once

#include "glew.h"

namespace render
{
namespace gl
{
class Shader final
{
public:
    explicit Shader(const GLenum type, const std::string& label = {})
            : m_handle{GL_ASSERT_FN( glCreateShader( type ) )}
            , m_type{type}
    {
        BOOST_ASSERT( type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER );
        BOOST_ASSERT( m_handle != 0 );

        if( !label.empty() )
        {
            GL_ASSERT( glObjectLabel( GL_SHADER, m_handle, -1, label.c_str() ) );
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

    GLenum getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const std::string& src)
    {
        const GLchar* data[1]{src.c_str()};
        GL_ASSERT( glShaderSource( m_handle, 1, data, nullptr ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const GLchar* src[], const GLsizei n)
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
        GLint success = GL_FALSE;
        GL_ASSERT( glGetShaderiv( m_handle, GL_COMPILE_STATUS, &success ) );
        return success == GL_TRUE;
    }

    std::string getInfoLog() const
    {
        GLint length = 0;
        GL_ASSERT( glGetShaderiv( m_handle, GL_INFO_LOG_LENGTH, &length ) );
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

    GLuint getHandle() const noexcept
    {
        return m_handle;
    }

private:
    const GLuint m_handle;

    const GLenum m_type;
};
}
}
