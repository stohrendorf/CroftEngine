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
            : BindableResource{glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays, GL_VERTEX_ARRAY, label}
            , m_indexBuffers{std::move( indexBuffers )}
            , m_vertexBuffers{std::move( vertexBuffers )}
    {
        bind();
        for( const auto& buffer : m_indexBuffers )
            buffer->bind();
        for( const auto& buffer : m_vertexBuffers )
            buffer->bind( program );
        unbind();
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


class VertexArrayBuilder final
{
public:
    VertexArrayBuilder& attach(const gsl::not_null<std::shared_ptr<IndexBuffer>>& buffer)
    {
        m_indexBuffers.emplace_back( buffer );
        return *this;
    }

    VertexArrayBuilder& attach(const std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>>& buffers)
    {
        copy( buffers.begin(), buffers.end(), back_inserter( m_indexBuffers ) );
        return *this;
    }

    VertexArrayBuilder& attach(const gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>& buffer)
    {
        m_vertexBuffers.emplace_back( buffer );
        return *this;
    }

    VertexArrayBuilder& attach(const std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>>& buffers)
    {
        copy( buffers.begin(), buffers.end(), back_inserter( m_vertexBuffers ) );
        return *this;
    }

    std::shared_ptr<VertexArray> build(const Program& program, const std::string& label = {})
    {
        return std::make_shared<VertexArray>( move( m_indexBuffers ), move( m_vertexBuffers ), program, label );
    }

private:
    std::vector<gsl::not_null<std::shared_ptr<IndexBuffer>>> m_indexBuffers;

    std::vector<gsl::not_null<std::shared_ptr<StructuredVertexBuffer>>> m_vertexBuffers;
};
}
}
