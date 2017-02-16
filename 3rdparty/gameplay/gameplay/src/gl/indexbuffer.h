#pragma once

#include "bindableresource.h"

namespace gameplay
{
namespace gl
{
class IndexBuffer : public BindableResource
{
public:
    explicit IndexBuffer()
        : BindableResource(glGenBuffers, [](GLuint handle)
                                       {
                                           glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
                                       }, glDeleteBuffers)
    {
    }
};
}
}
