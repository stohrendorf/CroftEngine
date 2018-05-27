#pragma once

#include "rendertarget.h"

#include <vector>

namespace gameplay
{
namespace gl
{
class Texture : public RenderTarget
{
public:
    explicit Texture(GLenum type, const std::string& label = {})
            : RenderTarget{glGenTextures, [type](GLuint handle) { glBindTexture( type, handle ); },
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
    void set(GLenum param, GLint value)
    {
        glTextureParameteri( getHandle(), param, value );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void set(GLenum param, GLfloat value)
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
    void image2D(GLint width, GLint height, bool generateMipmaps, GLint multisample = 0)
    {
        image2D( width, height, std::vector<T>{}, generateMipmaps, multisample );
    }

    template<typename T>
    void
    image2D(GLint width, GLint height, const std::vector<T>& data, bool generateMipmaps, GLint multisample = 0)
    {
        BOOST_ASSERT( width > 0 && height > 0 );
        BOOST_ASSERT(
                data.empty() || static_cast<std::size_t>(width) * static_cast<std::size_t>(height) == data.size() );

        bind();

        if( multisample > 0 )
            glTexImage2DMultisample( m_type, multisample, T::InternalFormat, width, height, GL_TRUE );
        else
            glTexImage2D( m_type, 0, T::InternalFormat, width, height, 0, T::Format, T::TypeId,
                          data.empty() ? nullptr : data.data() );
        checkGlError();

        m_width = width;
        m_height = height;

        // Set initial minification filter based on whether or not mipmaping was enabled.
        set( GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR );
        checkGlError();

        m_mipmap = generateMipmaps;

        if( m_mipmap )
        {
            glGenerateMipmap( m_type );
            checkGlError();
        }
    }

    void depthImage2D(GLint width, GLint height, GLint multisample = 0)
    {
        BOOST_ASSERT( width > 0 && height > 0 );

        bind();

        if( multisample > 0 )
            glTexImage2DMultisample( m_type, multisample, GL_DEPTH_COMPONENT, width, height, GL_TRUE );
        else
            glTexImage2D( m_type, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT,
                          GL_UNSIGNED_INT, nullptr );
        checkGlError();

        m_width = width;
        m_height = height;

        set( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        checkGlError();

        m_mipmap = false;
    }

private:
    const GLenum m_type;

    GLint m_width = -1;

    GLint m_height = -1;

    bool m_mipmap = false;
};
}
}
