#pragma once

#include "rendertarget.h"

#include <vector>

namespace render
{
namespace gl
{
class Texture : public RenderTarget
{
public:
    explicit Texture(GLenum type, const std::string& label = {})
            : RenderTarget{glGenTextures, [type](const GLuint handle) { glBindTexture( type, handle ); },
                           glDeleteTextures, GL_TEXTURE, label}
            , m_type{type}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setLabel(const std::string& lbl)
    {
        bind();
        glObjectLabel( GL_TEXTURE, getHandle(), static_cast<GLsizei>(lbl.length()), lbl.c_str() );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void set(const GLenum param, const GLint value)
    {
        glTextureParameteri( getHandle(), param, value );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void set(const GLenum param, const GLfloat value)
    {
        glTextureParameterf( getHandle(), param, value );
        checkGlError();
    }

    GLint getWidth() const noexcept override
    {
        return m_width;
    }

    GLint getHeight() const noexcept override
    {
        return m_height;
    }

    GLenum getType() const noexcept
    {
        return m_type;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void image2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT(
                data.empty() || static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        glTexImage2D( m_type, 0, T::InternalFormat, m_width, m_height, 0, T::Format, T::TypeId,
                      data.empty() ? nullptr : data.data() );
        checkGlError();

        if( m_mipmap )
        {
            glGenerateMipmap( m_type );
            checkGlError();
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void subImage2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT( static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        glTexSubImage2D( m_type, 0, 0, 0, m_width, m_height, T::Format, T::TypeId, data.data() );
        checkGlError();

        if( m_mipmap )
        {
            glGenerateMipmap( m_type );
            checkGlError();
        }
    }

    template<typename T>
    void image2D(GLint width, GLint height, bool generateMipmaps)
    {
        image2D( width, height, std::vector<T>{}, generateMipmaps );
    }

    template<typename T>
    void image2D(const GLint width, const GLint height, const std::vector<T>& data, const bool generateMipmaps)
    {
        BOOST_ASSERT( width > 0 && height > 0 );
        BOOST_ASSERT(
                data.empty() || static_cast<std::size_t>(width) * static_cast<std::size_t>(height) == data.size() );

        bind();

        glTexImage2D( m_type, 0, T::InternalFormat, width, height, 0, T::Format, T::TypeId,
                      data.empty() ? nullptr : data.data() );
        checkGlError();

        m_width = width;
        m_height = height;

        // Set initial minification filter based on whether or not mipmapping was enabled.
        set( GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR );
        checkGlError();

        m_mipmap = generateMipmaps;

        if( m_mipmap )
        {
            glGenerateMipmap( m_type );
            checkGlError();
        }
    }

    void depthImage2D(const GLint width, const GLint height)
    {
        BOOST_ASSERT( width > 0 && height > 0 );

        bind();

        glTexImage2D( m_type, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT,
                      GL_UNSIGNED_INT, nullptr );
        checkGlError();

        m_width = width;
        m_height = height;

        set( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        checkGlError();

        m_mipmap = false;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void copyImageSubData(const Texture& src)
    {
        if( m_type != src.m_type )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Refusing to copy image data with different types" ) );

        glCopyImageSubData( src.getHandle(), src.m_type, 0, 0, 0, 0, getHandle(), m_type, 0, 0, 0, 0, src.m_width,
                            src.m_height, 1 );
        checkGlError();
        m_width = src.m_width;
        m_height = src.m_height;
    }

private:
    const GLenum m_type;

    GLint m_width = -1;

    GLint m_height = -1;

    bool m_mipmap = false;
};
}
}
