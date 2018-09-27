#pragma once

namespace gameplay
{
namespace gl
{
class Shader final
{
public:
    explicit Shader(const GLenum type, const std::string& label = {})
            : m_handle{glCreateShader( type )}
            , m_type{type}
    {
        BOOST_ASSERT( type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER );
        checkGlError();
        BOOST_ASSERT( m_handle != 0 );

        if( !label.empty() )
        {
            glObjectLabel( GL_SHADER, m_handle, -1, label.c_str() );
            checkGlError();
        }
    }

    Shader(const Shader&) = delete;

    Shader(Shader&&) = delete;

    Shader& operator=(const Shader&) = delete;

    Shader& operator=(Shader&&) = delete;

    ~Shader()
    {
        glDeleteShader( m_handle );
        checkGlError();
    }

    GLenum getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const std::string& src)
    {
        const GLchar* data[1]{src.c_str()};
        glShaderSource( m_handle, 1, data, nullptr );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const GLchar* src[], const GLsizei n)
    {
        glShaderSource( m_handle, n, src, nullptr );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void compile()
    {
        glCompileShader( m_handle );
        checkGlError();
    }

    bool getCompileStatus() const
    {
        GLint success = GL_FALSE;
        glGetShaderiv( m_handle, GL_COMPILE_STATUS, &success );
        checkGlError();
        return success == GL_TRUE;
    }

    std::string getInfoLog() const
    {
        GLint length = 0;
        glGetShaderiv( m_handle, GL_INFO_LOG_LENGTH, &length );
        checkGlError();
        if( length == 0 )
        {
            length = 4096;
        }
        if( length > 0 )
        {
            const auto infoLog = new char[length];
            glGetShaderInfoLog( m_handle, length, nullptr, infoLog );
            checkGlError();
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
