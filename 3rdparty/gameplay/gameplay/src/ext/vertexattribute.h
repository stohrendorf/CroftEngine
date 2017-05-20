#pragma once

#include "gl/typetraits.h"


namespace gameplay
{
namespace ext
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
    explicit VertexAttribute(const U T::* member, bool normalized = false)
        : m_type{gl::TypeTraits<U>::TypeId}
        , m_pointer{&(static_cast<T*>(nullptr) ->* member)}
        , m_size{gl::TypeTraits<U>::ElementCount}
        , m_normalized{normalized}
        , m_stride{sizeof(T)}
    {
    }


    template<typename U>
    explicit VertexAttribute(const SingleAttribute<U>&, bool normalized = false)
        : m_type{gl::TypeTraits<U>::TypeId}
        , m_pointer{nullptr}
        , m_size{gl::TypeTraits<U>::ElementCount}
        , m_normalized{normalized}
        , m_stride{sizeof(U)}
    {
    }


    void bind(GLuint index) const
    {
        glVertexAttribPointer(index, m_size, m_type, m_normalized ? GL_TRUE : GL_FALSE, m_stride, m_pointer);
        gl::checkGlError();

        glEnableVertexAttribArray(index);
        gl::checkGlError();
    }


    GLsizei getStride() const noexcept
    {
        return m_stride;
    }


    uintptr_t getOffset() const noexcept
    {
        return reinterpret_cast<uintptr_t>(m_pointer);
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
