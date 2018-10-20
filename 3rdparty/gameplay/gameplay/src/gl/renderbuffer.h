#pragma once

#include "rendertarget.h"

namespace gameplay
{
namespace gl
{
class RenderBuffer : public RenderTarget
{
public:
    explicit RenderBuffer(const GLint width, const GLint height, const GLenum format, const std::string& label = {})
            : RenderTarget{glGenRenderbuffers,
                           [](const GLuint handle) { glBindRenderbuffer( GL_RENDERBUFFER, handle ); },
                           glDeleteRenderbuffers,
                           GL_RENDERBUFFER,
                           label}
            , m_width{width}
            , m_height{height}
            , m_format{format}
    {
        glRenderbufferStorage( GL_RENDERBUFFER, format, width, height );
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

    GLenum getFormat() const noexcept
    {
        return m_format;
    }

private:
    const GLint m_width;

    const GLint m_height;

    const GLenum m_format;
};
}
}
