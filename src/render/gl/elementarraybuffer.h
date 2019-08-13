#pragma once

#include "buffer.h"
#include "gsl-lite.hpp"
#include "typetraits.h"

#include <vector>

namespace render
{
namespace gl
{
template<typename T>
class ElementArrayBuffer : public Buffer
{
public:
    explicit ElementArrayBuffer(const std::string& label = {})
        : Buffer{::gl::BufferTargetARB::ElementArrayBuffer, label}
    {
    }

    T* map(const ::gl::BufferAccessARB access = ::gl::BufferAccessARB::ReadOnly)
    {
        bind();
        const void* data = GL_ASSERT_FN(::gl::mapBuffer(::gl::BufferTargetARB::ElementArrayBuffer, access));
        return static_cast<const T*>(data);
    }

    static void unmap()
    {
        GL_ASSERT(::gl::unmapBuffer(::gl::BufferTargetARB::ElementArrayBuffer));
    }

    void setData(const gsl::not_null<const T*>& data, const ::gl::core::SizeType size, const ::gl::BufferUsageARB usage)
    {
        Expects(size >= 0);

        bind();

        GL_ASSERT(::gl::bufferData(
            ::gl::BufferTargetARB::ElementArrayBuffer, gsl::narrow<std::size_t>(sizeof(T) * size), data.get(), usage));

        m_size = size;
    }

    void setData(const std::vector<T>& data, const ::gl::BufferUsageARB usage)
    {
        setData(gsl::not_null<const T*>(data.data()), gsl::narrow<::gl::core::SizeType>(data.size()), usage);
    }

    ::gl::core::SizeType size() const
    {
        return m_size;
    }

    void drawElements(::gl::PrimitiveType primitiveType) const
    {
        GL_ASSERT(::gl::drawElements(primitiveType, size(), gl::TypeTraits<T>::DrawElementsType, nullptr));
    }

private:
    ::gl::core::SizeType m_size = 0;
};
} // namespace gl
} // namespace render
