#pragma once

#include "bindableresource.h"

#include "gsl-lite.hpp"

namespace render
{
namespace gl
{
enum class ShaderType : RawGlEnum
{
    Compute = (RawGlEnum)::gl::GL_COMPUTE_SHADER,
    Vertex = (RawGlEnum)::gl::GL_VERTEX_SHADER,
    TessControl = (RawGlEnum)::gl::GL_TESS_CONTROL_SHADER,
    TessEvaluation = (RawGlEnum)::gl::GL_TESS_EVALUATION_SHADER,
    Geometry = (RawGlEnum)::gl::GL_GEOMETRY_SHADER,
    Fragment = (RawGlEnum)::gl::GL_FRAGMENT_SHADER,
};

enum class ShaderParameterName : RawGlEnum
{
    Type = (RawGlEnum)::gl::GL_SHADER_TYPE,
    DeleteStatus = (RawGlEnum)::gl::GL_DELETE_STATUS,
    CompileStatus = (RawGlEnum)::gl::GL_COMPILE_STATUS,
    InfoLogLength = (RawGlEnum)::gl::GL_INFO_LOG_LENGTH,
    SourceLength = (RawGlEnum)::gl::GL_SHADER_SOURCE_LENGTH,
};

class Shader final
{
public:
    explicit Shader(const ShaderType type, const std::string& label = {})
        : m_handle{ GL_ASSERT_FN( glCreateShader( (::gl::GLenum)type ) ) }
        , m_type{ type }
    {
        BOOST_ASSERT( type == ShaderType::Vertex || type == ShaderType::Fragment );
        Expects( m_handle != 0 );

        if( !label.empty() )
        {
            GL_ASSERT( glObjectLabel( (::gl::GLenum)ObjectIdentifier::Shader, m_handle, -1, label.c_str() ) );
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

    ShaderType getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setSource(const std::string& src)
    {
        const ::gl::GLchar* data[1]{ src.c_str() };
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
        GL_ASSERT( glGetShaderiv( m_handle, (::gl::GLenum)ShaderParameterName::CompileStatus, &success ) );
        return success == (::gl::GLint)::gl::GL_TRUE;
    }

    std::string getInfoLog() const
    {
        ::gl::GLint length = 0;
        GL_ASSERT( glGetShaderiv( m_handle, (::gl::GLenum)ShaderParameterName::InfoLogLength, &length ) );
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

    const ShaderType m_type;
};
}
}
