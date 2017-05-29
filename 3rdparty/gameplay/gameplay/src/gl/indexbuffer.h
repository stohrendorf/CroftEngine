#pragma once

#include "bindableresource.h"

#include <gsl/gsl>


namespace gameplay
{
    namespace gl
    {
        class IndexBuffer : public BindableResource
        {
        public:
            explicit IndexBuffer(const std::string& label = {})
                : BindableResource{glGenBuffers,
                    [](GLuint handle) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle); },
                    glDeleteBuffers,
                    GL_BUFFER,
                    label}
            {
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            const void* map()
            {
                bind();
                const void* data = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
                checkGlError();
                return data;
            }


            static void unmap()
            {
                glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                checkGlError();
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            template<typename T>
            void setData(const T* indexData, GLsizeiptr indexCount, bool dynamic)
            {
                Expects(indexData != nullptr);
                Expects(indexCount >= 0);

                bind();

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, gsl::narrow<GLsizeiptr>(sizeof(T) * indexCount), indexData, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
                checkGlError();

                m_indexCount = indexCount;
            }


            // ReSharper disable once CppMemberFunctionMayBeConst
            template<typename T>
            void setSubData(const T* indexData, GLintptr indexStart, GLsizeiptr indexCount)
            {
                Expects(indexData != nullptr);
                Expects(indexStart >= 0);
                Expects(indexCount >= 0);

                bind();

                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, gsl::narrow<GLintptr>(indexStart * sizeof(T)), gsl::narrow<GLsizeiptr>(indexCount * sizeof(T)), indexData);
                checkGlError();

                m_indexCount = indexCount;
            }


            template<typename T>
            void draw(GLenum mode) const
            {
                glDrawElements(mode, m_indexCount, TypeTraits<T>::TypeId, nullptr);
                checkGlError();
            }


            GLsizeiptr getIndexCount() const
            {
                return m_indexCount;
            }


        private:
            GLsizeiptr m_indexCount = 0;
        };
    }
}
