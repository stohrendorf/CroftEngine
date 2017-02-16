#pragma once

#include "bindableresource.h"

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


    static void unbindAll()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};
}
}
