#pragma once

#include "renderbuffer.h"
#include "texture.h"

namespace gameplay
{
namespace gl
{
class FrameBuffer : public BindableResource
{
public:
    explicit FrameBuffer()
        : BindableResource(glGenFramebuffers, [](GLuint handle)
                                            {
                                                glBindFramebuffer(GL_FRAMEBUFFER, handle);
                                            }, glDeleteFramebuffers)
    {
    }


    void attachTexture1D(GLenum attachment, const Texture& texture, GLint level = 0)
    {
        glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, attachment, texture.getType(), texture.getHandle(), level);
        checkGlError();
    }


    void attachTexture2D(GLenum attachment, const Texture& texture, GLint level = 0)
    {
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, texture.getType(), texture.getHandle(), level);
        checkGlError();
    }


    void attachTextureLayer(GLenum attachment, const Texture& texture, GLint level = 0, GLint layer = 0)
    {
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment, texture.getHandle(), level, layer);
        checkGlError();
    }


    void attachRenderbuffer(GLenum attachment, const RenderBuffer& renderBuffer)
    {
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer.getHandle());
        checkGlError();
    }


    bool isComplete() const
    {
        auto result = glCheckFramebufferStatus(getHandle()) == GL_FRAMEBUFFER_COMPLETE;
        checkGlError();
        return result;
    }


    static void unbindAll()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        checkGlError();
    }
};
}
}
