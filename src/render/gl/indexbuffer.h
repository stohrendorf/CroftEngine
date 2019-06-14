#pragma once

#include "bindableresource.h"
#include "typetraits.h"

#include "gsl-lite.hpp"

#include <vector>

namespace render
{
namespace gl
{
class IndexBuffer : public BindableResource
{
public:
    explicit IndexBuffer(const std::string& label = {})
            : BindableResource{::gl::glGenBuffers,
                               [](const ::gl::GLuint handle) {
                                   ::gl::glBindBuffer( ::gl::GL_ELEMENT_ARRAY_BUFFER, handle );
                               },
                               ::gl::glDeleteBuffers,
                               ::gl::GL_BUFFER,
                               label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        const void* data = GL_ASSERT_FN( glMapBuffer( ::gl::GL_ELEMENT_ARRAY_BUFFER, ::gl::GL_READ_ONLY ) );
        return data;
    }

    static void unmap()
    {
        GL_ASSERT( glUnmapBuffer( ::gl::GL_ELEMENT_ARRAY_BUFFER ) );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setData(const gsl::not_null<const T*>& indexData, const ::gl::GLsizei indexCount, const bool dynamic)
    {
        Expects( indexCount >= 0 );

        bind();

        GL_ASSERT( glBufferData( ::gl::GL_ELEMENT_ARRAY_BUFFER,
                                 gsl::narrow<::gl::GLsizeiptr>( sizeof( T ) * indexCount ),
                                 indexData.get(),
                                 dynamic ? ::gl::GL_DYNAMIC_DRAW : ::gl::GL_STATIC_DRAW ) );

        m_indexCount = indexCount;
        m_storageType = TypeTraits<T>::TypeId;
    }

    template<typename T>
    void setData(const std::vector<T>& data, bool dynamic)
    {
        setData( gsl::not_null<const T*>( data.data() ), gsl::narrow<::gl::GLsizei>( data.size() ), dynamic );
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setSubData(const gsl::not_null<const T*>& indexData, ::gl::GLsizei indexStart, ::gl::GLsizei indexCount)
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

        GL_ASSERT( glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, gsl::narrow<GLintptr>( indexStart * sizeof( T ) ),
                                    gsl::narrow<GLsizeiptr>( indexCount * sizeof( T ) ), indexData.get() ) );
    }

    void draw(const ::gl::GLenum mode) const
    {
        GL_ASSERT( glDrawElements( mode, m_indexCount, m_storageType, nullptr ) );
    }

    ::gl::GLsizei getIndexCount() const
    {
        return m_indexCount;
    }

    ::gl::GLenum getStorageType() const
    {
        return m_storageType;
    }

private:
    ::gl::GLsizei m_indexCount = 0;

    ::gl::GLenum m_storageType = ::gl::GL_NONE;
};
}
}
