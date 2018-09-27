#pragma once

#include "typetraits.h"

namespace gameplay
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
    explicit VertexAttribute(const U T::* member, const bool normalized = false)
            : m_type{TypeTraits<U>::TypeId}
            , m_pointer{&(static_cast<T*>(nullptr)->*member)}
            , m_size{TypeTraits<U>::ElementCount}
            , m_normalized{normalized}
            , m_stride{sizeof( T )}
    {
    }

    template<typename U>
    explicit VertexAttribute(const SingleAttribute<U>&, const bool normalized = false)
            : m_type{TypeTraits<U>::TypeId}
            , m_pointer{nullptr}
            , m_size{TypeTraits<U>::ElementCount}
            , m_normalized{normalized}
            , m_stride{sizeof( U )}
    {
    }

    void bind(const GLuint index) const
    {
        glVertexAttribPointer( index, m_size, m_type, m_normalized ? GL_TRUE : GL_FALSE, m_stride, m_pointer );
        checkGlError();

        glEnableVertexAttribArray( index );
        checkGlError();
    }

    GLsizei getStride() const noexcept
    {
        return m_stride;
    }

    std::uintptr_t getOffset() const noexcept
    {
        return reinterpret_cast<std::uintptr_t>(m_pointer);
    }

private:
    const GLenum m_type;

    const GLvoid* const m_pointer;

    const GLint m_size;

    const bool m_normalized;

    const GLsizei m_stride;
};
}
}
