#pragma once

#include "bindableresource.h"

namespace gameplay
{
namespace gl
{
class VertexBuffer : public BindableResource
{
public:
    explicit VertexBuffer()
        : BindableResource(glGenBuffers, [](GLuint handle)
                                       {
                                           glBindBuffer(GL_ARRAY_BUFFER, handle);
                                       }, glDeleteBuffers)
    {
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        auto data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        checkGlError();
        return data;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void* mapRw()
    {
        bind();
        auto data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        checkGlError();
        return data;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    // ReSharper disable once CppMemberFunctionMayBeStatic
    void unmap()
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
        checkGlError();
    }
};
}
}
