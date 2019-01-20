#pragma once

#include "bindableresource.h"
#include "typetraits.h"

#include "gsl-lite.hpp"

#include <vector>

namespace gameplay
{
namespace gl
{
class IndexBuffer : public BindableResource
{
public:
    explicit IndexBuffer(const std::string& label = {})
            : BindableResource{glGenBuffers,
                               [](const GLuint handle) { glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, handle ); },
                               glDeleteBuffers,
                               GL_BUFFER,
                               label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        const void* data = glMapBuffer( GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY );
        checkGlError();
        return data;
    }

    static void unmap()
    {
        glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
        checkGlError();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setData(const gsl::not_null<const T*>& indexData, const GLsizei indexCount, const bool dynamic)
    {
        Expects( indexCount >= 0 );

        bind();

        glBufferData( GL_ELEMENT_ARRAY_BUFFER, gsl::narrow<GLsizeiptr>( sizeof( T ) * indexCount ), indexData.get(),
                      dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW );
        checkGlError();

        m_indexCount = indexCount;
        m_storageType = TypeTraits<T>::TypeId;
    }

    template<typename T>
    void setData(const std::vector<T>& data, bool dynamic)
    {
        setData( gsl::not_null<const T*>( data.data() ), gsl::narrow<GLsizei>( data.size() ), dynamic );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setSubData(const gsl::not_null<const T*>& indexData, GLsizei indexStart, GLsizei indexCount)
    {
        Expects( indexStart >= 0 );
        Expects( indexCount >= 0 );
        if( indexStart + indexCount > m_indexCount )
        {
            BOOST_THROW_EXCEPTION( std::out_of_range{"Sub-range exceeds buffer range"} );
        }

        if( TypeTraits<T>::TypeId != m_storageType )
        {
            BOOST_THROW_EXCEPTION( std::logic_error{"Incompatible storage type for buffer sub-data"} );
        }

        bind();

        glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, gsl::narrow<GLintptr>( indexStart * sizeof( T ) ),
                         gsl::narrow<GLsizeiptr>( indexCount * sizeof( T ) ), indexData.get() );
        checkGlError();
    }

    void draw(const GLenum mode) const
    {
        glDrawElements( mode, m_indexCount, m_storageType, nullptr );
        checkGlError();
    }

    GLsizei getIndexCount() const
    {
        return m_indexCount;
    }

    const GLenum& getStorageType() const
    {
        return m_storageType;
    }

private:
    GLsizei m_indexCount = 0;

    GLenum m_storageType = GL_NONE;
};
}
}
