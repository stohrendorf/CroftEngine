#pragma once

#include "bindableresource.h"

namespace gameplay
{
namespace gl
{
class RenderBuffer : public BindableResource
{
public:
    explicit RenderBuffer()
        : BindableResource(glGenRenderbuffers, [](GLuint handle)
                                             {
                                                 glBindRenderbuffer(GL_RENDERBUFFER, handle);
                                             }, glDeleteRenderbuffers)
    {
    }
};
}
}
