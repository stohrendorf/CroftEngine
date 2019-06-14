#pragma once

#include "rendertarget.h"

namespace render
{
namespace gl
{
class RenderBuffer : public RenderTarget
{
public:
    explicit RenderBuffer(const ::gl::GLint width, const ::gl::GLint height, const ::gl::GLenum format,
                          const std::string& label = {})
            : RenderTarget{::gl::glGenRenderbuffers,
                           [](const ::gl::GLuint handle) { ::gl::glBindRenderbuffer( ::gl::GL_RENDERBUFFER, handle ); },
                           ::gl::glDeleteRenderbuffers,
                           ::gl::GL_RENDERBUFFER,
                           label}
            , m_width{width}
            , m_height{height}
            , m_format{format}
    {
        GL_ASSERT( glRenderbufferStorage( ::gl::GL_RENDERBUFFER, format, width, height ) );
    }

    ::gl::GLint getWidth() const noexcept override
    {
        return m_width;
    }

    ::gl::GLint getHeight() const noexcept override
    {
        return m_height;
    }

    ::gl::GLenum getFormat() const noexcept
    {
        return m_format;
    }

private:
    const ::gl::GLint m_width;

    const ::gl::GLint m_height;

    const ::gl::GLenum m_format;
};
}
}
