#pragma once

#include "typetraits.h"

namespace render
{
namespace gl
{
template<typename VertexT>
class VertexAttribute
{
public:
    struct Single
    {
    };

    template<typename U>
    VertexAttribute(const U VertexT::*member, const bool normalized = false)
        : m_type{TypeTraits<U>::VertexAttribPointerType}
        , m_pointer{&(static_cast<VertexT*>(nullptr)->*member)}
        , m_size{TypeTraits<U>::ElementCount}
        , m_normalized{normalized}
    {
    }

    VertexAttribute(const Single&, const bool normalized = false)
        : m_type{TypeTraits<VertexT>::VertexAttribPointerType}
        , m_pointer{nullptr}
        , m_size{TypeTraits<VertexT>::ElementCount}
        , m_normalized{normalized}
    {
    }

    void bind(const uint32_t index) const
    {
        GL_ASSERT(::gl::vertexAttribPointer(index, m_size, m_type, m_normalized, sizeof(VertexT), m_pointer));
        GL_ASSERT(::gl::enableVertexAttribArray(index));
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
};
} // namespace gl
} // namespace render
