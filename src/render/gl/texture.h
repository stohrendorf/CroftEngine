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
    explicit Texture(::gl::GLenum type, const std::string& label = {})
        : RenderTarget{ ::gl::glGenTextures, [type](const ::gl::GLuint handle) { glBindTexture( type, handle ); },
                        ::gl::glDeleteTextures, ::gl::GL_TEXTURE, label
    }
          , m_type{ type }
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void setLabel(const std::string& lbl)
    {
        bind();
        GL_ASSERT(
            glObjectLabel( ::gl::GL_TEXTURE, getHandle(), static_cast<::gl::GLsizei>(lbl.length()), lbl.c_str() ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    Texture& set(const ::gl::GLenum param, const ::gl::GLint value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), param, value ) );
        return *this;
    }

    Texture& set(const ::gl::GLenum param, const ::gl::GLenum value)
    {
        GL_ASSERT( glTextureParameteri( getHandle(), param, value ) );
        return *this;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    Texture& set(const ::gl::GLenum param, const ::gl::GLfloat value)
    {
        GL_ASSERT( glTextureParameterf( getHandle(), param, value ) );
        return *this;
    }

    ::gl::GLint getWidth() const noexcept override
    {
        return m_width;
    }

    ::gl::GLint getHeight() const noexcept override
    {
        return m_height;
    }

    ::gl::GLenum getType() const noexcept
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

        GL_ASSERT( glTexImage2D( m_type, 0, T::InternalFormat, m_width, m_height, 0, T::Format, T::TypeId,
                                 data.empty() ? nullptr : data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void subImage2D(const std::vector<T>& data)
    {
        BOOST_ASSERT( m_width > 0 && m_height > 0 );
        BOOST_ASSERT( static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height) == data.size() );

        bind();

        GL_ASSERT( glTexSubImage2D( m_type, 0, 0, 0, m_width, m_height, T::Format, T::TypeId, data.data() ) );

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }
    }

    template<typename T>
    Texture& image2D(::gl::GLint width, ::gl::GLint height, bool generateMipmaps)
    {
        return image2D( width, height, std::vector<T>{}, generateMipmaps );
    }

    template<typename T>
    Texture&
    image2D(const ::gl::GLint width, const ::gl::GLint height, const std::vector<T>& data, const bool generateMipmaps)
    {
        BOOST_ASSERT( width > 0 && height > 0 );
        BOOST_ASSERT(
            data.empty() || static_cast<std::size_t>(width) * static_cast<std::size_t>(height) == data.size() );

        bind();

        GL_ASSERT( glTexImage2D( m_type, 0, T::InternalFormat, width, height, 0, T::Format, T::TypeId,
                                 data.empty() ? nullptr : data.data() ) );

        m_width = width;
        m_height = height;
        m_mipmap = generateMipmaps;

        if( m_mipmap )
        {
            GL_ASSERT( glGenerateMipmap( m_type ) );
        }

        return *this;
    }

    void depthImage2D(const ::gl::GLint width, const ::gl::GLint height)
    {
        BOOST_ASSERT( width > 0 && height > 0 );

        bind();

        GL_ASSERT( glTexImage2D( m_type, 0, ::gl::GL_DEPTH_COMPONENT24, width, height, 0, ::gl::GL_DEPTH_COMPONENT,
                                 ::gl::GL_UNSIGNED_INT, nullptr ) );

        m_width = width;
        m_height = height;
        m_mipmap = false;
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void copyImageSubData(const Texture& src)
    {
        if( m_type != src.m_type )
            BOOST_THROW_EXCEPTION( std::runtime_error( "Refusing to copy image data with different types" ) );

        GL_ASSERT( glCopyImageSubData( src.getHandle(), src.m_type, 0, 0, 0, 0, getHandle(), m_type, 0, 0, 0, 0,
                                       src.m_width,
                                       src.m_height, 1 ) );
        m_width = src.m_width;
        m_height = src.m_height;
    }

private:
    const ::gl::GLenum m_type;

    ::gl::GLint m_width = -1;

    ::gl::GLint m_height = -1;

    bool m_mipmap = false;
};
}
}
