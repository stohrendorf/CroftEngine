#pragma once

#include "typetraits.h"

namespace render
{
namespace gl
{
class VertexAttribute
{
public:
    template<typename T>
    struct SingleAttribute
    {
        T attribute;
    };

    template<typename T, typename U>
    explicit VertexAttribute(const U T::*member, const bool normalized = false)
        : m_type{TypeTraits<U>::VertexAttribPointerType}
        , m_pointer{&(static_cast<T*>(nullptr)->*member)}
        , m_size{TypeTraits<U>::ElementCount}
        , m_normalized{normalized}
        , m_stride{sizeof(T)}
    {
    }

    template<typename U>
    explicit VertexAttribute(const SingleAttribute<U>&, const bool normalized = false)
        : m_type{TypeTraits<U>::VertexAttribPointerType}
        , m_pointer{nullptr}
        , m_size{TypeTraits<U>::ElementCount}
        , m_normalized{normalized}
        , m_stride{sizeof(U)}
    {
    }

    void bind(const uint32_t index) const
    {
        GL_ASSERT(::gl::vertexAttribPointer( index, m_size, m_type, m_normalized, m_stride, m_pointer));
        GL_ASSERT(::gl::enableVertexAttribArray(index));
    }

    auto getStride() const noexcept
    {
        return m_stride;
    }

    std::uintptr_t getOffset() const noexcept
    {
        return reinterpret_cast<std::uintptr_t>(m_pointer);
    }

private:
    const ::gl::VertexAttribPointerType m_type;

    const void* const m_pointer;

    const int32_t m_size;

    const bool m_normalized;

    const ::gl::core::SizeType m_stride;
};
} // namespace gl
} // namespace render
