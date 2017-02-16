#pragma once

#include "rendertarget.h"

namespace gameplay
{
namespace gl
{
class RenderBuffer : public RenderTarget
{
public:
    explicit RenderBuffer(GLint width, GLint height, GLenum format, GLsizei multisample = 0)
        : RenderTarget(glGenRenderbuffers, [](GLuint handle)
                                             {
                                                 glBindRenderbuffer(GL_RENDERBUFFER, handle);
                                             }, glDeleteRenderbuffers)
        , m_width(width)
        , m_height(height)
        , m_format(format)
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample, format, width, height);
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
