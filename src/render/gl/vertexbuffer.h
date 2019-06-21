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
        : BindableResource{
            ::gl::genBuffers,
            [](const uint32_t handle) { ::gl::bindBuffer(::gl::BufferTargetARB::ArrayBuffer, handle); },
            ::gl::deleteBuffers,
            ::gl::ObjectIdentifier::Buffer,
            label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        return GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ArrayBuffer, ::gl::BufferAccessARB::ReadOnly));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void* mapRw()
    {
        bind();
        return GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ArrayBuffer, ::gl::BufferAccessARB::ReadWrite));
    }

    static void unmap()
    {
        GL_ASSERT(::gl::unmapBuffer(::gl::BufferTargetARB::ArrayBuffer));
    }
};
} // namespace gl
} // namespace render
