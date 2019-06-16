#pragma once

#include "bindableresource.h"
#include "gsl-lite.hpp"
#include "typetraits.h"

#include <vector>

namespace render
{
namespace gl
{
enum class PrimitiveType : RawGlEnum
{
    Lines = (RawGlEnum)::gl::GL_LINES,
    LinesAdjacency = (RawGlEnum)::gl::GL_LINES_ADJACENCY,
    LineLoop = (RawGlEnum)::gl::GL_LINE_LOOP,
    LineStrip = (RawGlEnum)::gl::GL_LINE_STRIP,
    LineStripAdjacency = (RawGlEnum)::gl::GL_LINE_STRIP_ADJACENCY,
    Patches = (RawGlEnum)::gl::GL_PATCHES,
    Points = (RawGlEnum)::gl::GL_POINTS,
    Polygon = (RawGlEnum)::gl::GL_POLYGON,
    Quads = (RawGlEnum)::gl::GL_QUADS,
    QuadStrip = (RawGlEnum)::gl::GL_QUAD_STRIP,
    Triangles = (RawGlEnum)::gl::GL_TRIANGLES,
    TrianglesAdjacency = (RawGlEnum)::gl::GL_TRIANGLES_ADJACENCY,
    Fan = (RawGlEnum)::gl::GL_TRIANGLE_FAN,
    TriangleStrip = (RawGlEnum)::gl::GL_TRIANGLE_STRIP,
    TriangleStripAdjacency = (RawGlEnum)::gl::GL_TRIANGLE_STRIP_ADJACENCY,
};

class IndexBuffer : public BindableResource
{
public:
    explicit IndexBuffer(const std::string& label = {})
        : BindableResource{::gl::glGenBuffers,
                           [](const ::gl::GLuint handle) { ::gl::glBindBuffer(::gl::GL_ELEMENT_ARRAY_BUFFER, handle); },
                           ::gl::glDeleteBuffers,
                           ObjectIdentifier::Buffer,
                           label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    const void* map()
    {
        bind();
        const void* data = GL_ASSERT_FN(glMapBuffer(::gl::GL_ELEMENT_ARRAY_BUFFER, ::gl::GL_READ_ONLY));
        return data;
    }

    static void unmap()
    {
        GL_ASSERT(glUnmapBuffer(::gl::GL_ELEMENT_ARRAY_BUFFER));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setData(const gsl::not_null<const T*>& indexData, const ::gl::GLsizei indexCount, const bool dynamic)
    {
        Expects(indexCount >= 0);

        bind();

        GL_ASSERT(glBufferData(::gl::GL_ELEMENT_ARRAY_BUFFER,
                               gsl::narrow<::gl::GLsizeiptr>(sizeof(T) * indexCount),
                               indexData.get(),
                               dynamic ? ::gl::GL_DYNAMIC_DRAW : ::gl::GL_STATIC_DRAW));

        m_indexCount = indexCount;
        m_storageType = TypeTraits<T>::DrawElementsType;
    }

    template<typename T>
    void setData(const std::vector<T>& data, bool dynamic)
    {
        setData(gsl::not_null<const T*>(data.data()), gsl::narrow<::gl::GLsizei>(data.size()), dynamic);
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    template<typename T>
    void setSubData(const gsl::not_null<const T*>& indexData, ::gl::GLsizei indexStart, ::gl::GLsizei indexCount)
    {
        Expects(indexStart >= 0);
        Expects(indexCount >= 0);
        if(indexStart + indexCount > m_indexCount)
        {
            BOOST_THROW_EXCEPTION(std::out_of_range{"Sub-range exceeds buffer range"});
        }

        if(!m_storageType.is_initialized())
        {
            BOOST_THROW_EXCEPTION(std::logic_error{"Buffer is not initialized"});
        }

        if(TypeTraits<T>::DrawElementsType != m_storageType)
        {
            BOOST_THROW_EXCEPTION(std::logic_error{"Incompatible storage type for buffer sub-data"});
        }

        bind();

        GL_ASSERT(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                                  gsl::narrow<GLintptr>(indexStart * sizeof(T)),
                                  gsl::narrow<GLsizeiptr>(indexCount * sizeof(T)),
                                  indexData.get()));
    }

    void draw(const PrimitiveType mode) const
    {
        Expects(m_storageType.is_initialized());
        GL_ASSERT(glDrawElements((::gl::GLenum)mode, m_indexCount, (::gl::GLenum)*m_storageType, nullptr));
    }

    ::gl::GLsizei getIndexCount() const
    {
        return m_indexCount;
    }

private:
    ::gl::GLsizei m_indexCount = 0;

    boost::optional<DrawElementsType> m_storageType;
};
} // namespace gl
} // namespace render
