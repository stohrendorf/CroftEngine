#pragma once

#include "bindableresource.h"
#include "indexbuffer.h"
#include "program.h"
#include "structuredvertexbuffer.h"

#include <utility>
#include <vector>

namespace render
{
namespace gl
{
class VertexArray : public BindableResource
{
public:
    explicit VertexArray(std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>> indexBuffers,
                         std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>> vertexBuffers,
                         const Program& program,
                         const std::string& label = {})
        : BindableResource{::gl::genVertexArrays,
                           ::gl::bindVertexArray,
                           ::gl::deleteVertexArrays,
                           ::gl::ObjectIdentifier::VertexArray,
                           label}
        , m_indexBuffers{std::move(indexBuffers)}
        , m_vertexBuffers{std::move(vertexBuffers)}
    {
        bind();
        for(const auto& buffer : m_indexBuffers)
            buffer->bind();
        for(const auto& buffer : m_vertexBuffers)
            buffer->bind(program);
        unbind();
    }

    explicit VertexArray(const gsl::not_null<std::shared_ptr<IndexBuffer>>& indexBuffer,
                         const gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>& vertexBuffer,
                         const Program& program,
                         const std::string& label = {})
        : VertexArray{std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>>{indexBuffer},
                      std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>>{vertexBuffer},
                      program,
                      label}
    {
    }

    const std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>>& getIndexBuffers() const
    {
        return m_indexBuffers;
    }

    const std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>>& getVertexBuffers() const
    {
        return m_vertexBuffers;
    }

private:
    std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>> m_indexBuffers;

    std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>> m_vertexBuffers;
};
} // namespace gl
} // namespace render
