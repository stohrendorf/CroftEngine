#pragma once

#include "gl/util.h"

namespace gameplay
{
namespace ext
{

class VertexAttribute
{
public:
    template<typename T, typename U>
    explicit VertexAttribute(GLenum type, const U T::* member, GLint size = 1, bool normalized = false)
        : m_type{ type }
        , m_pointer{&(static_cast<T*>(nullptr)->*member)}
        , m_size{size}
        , m_normalized{normalized}
        , m_stride{sizeof(T)}
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
