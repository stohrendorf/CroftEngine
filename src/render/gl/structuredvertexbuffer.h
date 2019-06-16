#pragma once

#include "gsl-lite.hpp"
#include "program.h"
#include "vertexattribute.h"
#include "vertexbuffer.h"

#include <map>

namespace render
{
namespace gl
{
class StructuredVertexBuffer : public VertexBuffer
{
public:
    using AttributeMapping = std::map<std::string, VertexAttribute>;

    explicit StructuredVertexBuffer(const AttributeMapping& mapping, const bool dynamic, const std::string& label = {})
        : VertexBuffer{label}
        , m_mapping{mapping}
        , m_dynamic{dynamic}
        , m_vertexCount{0}
    {
        BOOST_ASSERT(!mapping.empty());

        m_size = -1;
        for(const auto& attribute : mapping)
        {
            const auto tmp = attribute.second.getStride();
            if(m_size != -1 && m_size != tmp)
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("Inconsistent stride in vertex attribute mapping"));
            }

            m_size = tmp;
        }

        BOOST_ASSERT(m_size > 0);
    }

    void bind(const Program& program) const
    {
        VertexBuffer::bind();

        for(const auto& attribute : program.getActiveAttributes())
        {
            auto it = m_mapping.find(attribute.getName());
            if(it == m_mapping.end())
                continue;

            it->second.bind(gsl::narrow<::gl::GLuint>(attribute.getLocation()));
        }
    }

    template<typename T>
    void assignSub(const gsl::not_null<const T*>& vertexData, const size_t vertexStart, size_t vertexCount)
    {
        if(sizeof(T) != m_size)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument(
                "Trying to assign vertex data which has a different size than specified in the format"));
        }

        VertexBuffer::bind();

        if(vertexCount == 0)
        {
            vertexCount = m_vertexCount - vertexStart;
        }

        GL_ASSERT(glBufferSubData(::gl::GL_ARRAY_BUFFER, vertexStart * m_size, vertexCount * m_size, vertexData));
    }

    template<typename T>
    void assign(const gsl::not_null<const T*>& vertexData, const size_t vertexCount)
    {
        if(sizeof(T) != m_size)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument(
                "Trying to assign vertex data which has a different size than specified in the format"));
        }

        VertexBuffer::bind();

        if(vertexCount != 0)
            m_vertexCount = vertexCount;

        GL_ASSERT(glBufferData(::gl::GL_ARRAY_BUFFER,
                               m_size * m_vertexCount,
                               vertexData,
                               m_dynamic ? ::gl::GL_DYNAMIC_DRAW : ::gl::GL_STATIC_DRAW));
    }

    template<typename T>
    void assignRaw(const gsl::not_null<const T*>& vertexData, const size_t vertexCount)
    {
        VertexBuffer::bind();

        if(vertexCount != 0)
            m_vertexCount = vertexCount;

        GL_ASSERT(glBufferData(::gl::GL_ARRAY_BUFFER,
                               m_size * m_vertexCount,
                               vertexData,
                               m_dynamic ? ::gl::GL_DYNAMIC_DRAW : ::gl::GL_STATIC_DRAW));
    }

    template<typename T>
    void assign(const std::vector<T>& data)
    {
        if(!data.empty())
            assign<T>(data.data(), data.size());
    }

    template<typename T>
    void assignRaw(const std::vector<T>& data, const size_t vertexCount)
    {
        if(!data.empty())
            assignRaw<T>(data.data(), vertexCount);
    }

    void reserve(const size_t n)
    {
        m_vertexCount = n;
        GL_ASSERT(glBufferData(::gl::GL_ARRAY_BUFFER,
                               m_size * m_vertexCount,
                               nullptr,
                               m_dynamic ? ::gl::GL_DYNAMIC_DRAW : ::gl::GL_STATIC_DRAW));
    }

    template<typename T>
    T* mapTypedRw()
    {
        if(sizeof(T) != m_size)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument(
                "Trying to map vertex data which has a different size than specified in the format"));
        }

        return static_cast<T*>(mapRw());
    }

    template<typename T>
    const T* mapTyped()
    {
        if(sizeof(T) != m_size)
        {
            BOOST_THROW_EXCEPTION(std::invalid_argument(
                "Trying to map vertex data which has a different size than specified in the format"));
        }

        return static_cast<T*>(map());
    }

    size_t getVertexCount() const noexcept
    {
        return m_vertexCount;
    }

    bool isDynamic() const noexcept
    {
        return m_dynamic;
    }

    ::gl::GLsizei getVertexSize() const noexcept
    {
        return m_size;
    }

    const AttributeMapping& getAttributeMapping() const
    {
        return m_mapping;
    }

private:
    const AttributeMapping m_mapping;

    ::gl::GLsizei m_size = -1;

    bool m_dynamic;

    size_t m_vertexCount;
};
} // namespace gl
} // namespace render
