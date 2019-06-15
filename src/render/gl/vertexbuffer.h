#pragma once

#include "bindableresource.h"

namespace render
{
namespace gl
{
class VertexBuffer : public BindableResource
{
public:
    explicit VertexBuffer(const std::string& label = {})
        : BindableResource{ ::gl::glGenBuffers,
                            [](const ::gl::GLuint handle) { ::gl::glBindBuffer( ::gl::GL_ARRAY_BUFFER, handle ); },
                            ::gl::glDeleteBuffers,
                            ::gl::GL_BUFFER,
                            label
    }
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        return GL_ASSERT_FN( glMapBuffer( ::gl::GL_ARRAY_BUFFER, ::gl::GL_READ_ONLY ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void* mapRw()
    {
        bind();
        return GL_ASSERT_FN( glMapBuffer( ::gl::GL_ARRAY_BUFFER, ::gl::GL_READ_WRITE ) );
    }

    static void unmap()
    {
        GL_ASSERT( glUnmapBuffer( ::gl::GL_ARRAY_BUFFER ) );
    }
};
}
}
