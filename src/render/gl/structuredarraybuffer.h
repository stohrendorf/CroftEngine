#pragma once

#include "arraybuffer.h"
#include "gsl-lite.hpp"
#include "program.h"
#include "vertexattribute.h"

#include <map>

namespace render
{
namespace gl
{
template<typename VertexT>
using VertexAttributeMapping = std::map<std::string, VertexAttribute<VertexT>>;

template<typename VertexT>
class StructuredArrayBuffer : public ArrayBuffer<VertexT>
{
public:
    explicit StructuredArrayBuffer(const VertexAttributeMapping<VertexT>& mapping, const std::string& label = {})
        : ArrayBuffer{label}
        , m_mapping{mapping}
    {
        BOOST_ASSERT(!mapping.empty());
    }

    void bind(const Program& program) const
    {
        ArrayBuffer::bind();

        for(const auto& attribute : program.getActiveAttributes())
        {
            auto it = m_mapping.find(attribute.getName());
            if(it == m_mapping.end())
                continue;

            it->second.bind(attribute.getLocation());
        }
    }

    void setSubData(const gsl::not_null<const VertexT*>& data, const size_t start, size_t count)
    {
        ArrayBuffer::bind();

        if(count == 0)
        {
            count = size() - start;
        }

        GL_ASSERT(::gl::bufferSubData(
            ::gl::BufferTargetARB::ArrayBuffer, start * sizeof(VertexT), count * sizeof(VertexT), data));
    }

    void setData(const gsl::not_null<const VertexT*>& data, const size_t count, const ::gl::BufferUsageARB access)
    {
        ArrayBuffer::bind();

        if(count != 0)
            m_size = gsl::narrow<::gl::core::SizeType>(count);

        GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ArrayBuffer, sizeof(VertexT) * size(), data, access));
    }

    void setDataRaw(const gsl::not_null<const VertexT*>& data, const size_t count, const ::gl::BufferUsageARB access)
    {
        ArrayBuffer::bind();

        if(count != 0)
            m_size = count;

        GL_ASSERT(::gl::bufferData(::gl::BufferTargetARB::ArrayBuffer, sizeof(VertexT) * size(), data, access));
    }

    void setData(const std::vector<VertexT>& data, const ::gl::BufferUsageARB access)
    {
        if(!data.empty())
            setData(data.data(), data.size(), access);
    }

    void setDataRaw(const std::vector<VertexT>& data, const size_t count, const ::gl::BufferUsageARB access)
    {
        if(!data.empty())
            setDataRaw(data.data(), count, access);
    }

    const VertexAttributeMapping<VertexT>& getAttributeMapping() const
    {
        return m_mapping;
    }

private:
    const VertexAttributeMapping<VertexT> m_mapping;
};
} // namespace gl
} // namespace render
