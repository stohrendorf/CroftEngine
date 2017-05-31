#pragma once

#include "bindableresource.h"

#include "indexbuffer.h"
#include "program.h"

#include "ext/structuredvertexbuffer.h"

#include <vector>


namespace gameplay
{
    namespace gl
    {
        class VertexArray : public BindableResource
        {
        public:
            explicit VertexArray(const std::vector<std::shared_ptr<IndexBuffer>>& indexBuffers,
                                 const std::vector<std::shared_ptr<ext::StructuredVertexBuffer>>& vertexBuffers,
                                 const Program& program,
                                 const std::string& label = {})
                : BindableResource{glGenVertexArrays, glBindVertexArray, glDeleteVertexArrays, GL_VERTEX_ARRAY, label}
                , m_indexBuffers{indexBuffers}
                , m_vertexBuffers{vertexBuffers}
            {
                bind();
                for( const auto& buffer : m_indexBuffers )
                    buffer->bind();
                for( const auto& buffer : m_vertexBuffers )
                    buffer->bind(program);
                unbind();
            }


            const std::vector<std::shared_ptr<IndexBuffer>>& getIndexBuffers() const
            {
                return m_indexBuffers;
            }


            const std::vector<std::shared_ptr<ext::StructuredVertexBuffer>>& getVertexBuffers() const
            {
                return m_vertexBuffers;
            }


        private:
            std::vector<std::shared_ptr<IndexBuffer>> m_indexBuffers;

            std::vector<std::shared_ptr<ext::StructuredVertexBuffer>> m_vertexBuffers;
        };


        class VertexArrayBuilder final
        {
        public:
            VertexArrayBuilder& attach(const std::shared_ptr<IndexBuffer>& buffer)
            {
                m_indexBuffers.emplace_back(buffer);
                return *this;
            }


            VertexArrayBuilder& attach(const std::vector<std::shared_ptr<IndexBuffer>>& buffers)
            {
                copy(buffers.begin(), buffers.end(), back_inserter(m_indexBuffers));
                return *this;
            }


            VertexArrayBuilder& attach(const std::shared_ptr<ext::StructuredVertexBuffer>& buffer)
            {
                m_vertexBuffers.emplace_back(buffer);
                return *this;
            }


            VertexArrayBuilder& attach(const std::vector<std::shared_ptr<ext::StructuredVertexBuffer>>& buffers)
            {
                copy(buffers.begin(), buffers.end(), back_inserter(m_vertexBuffers));
                return *this;
            }


            std::shared_ptr<VertexArray> build(const Program& program, const std::string& label = {})
            {
                return std::make_shared<VertexArray>(move(m_indexBuffers), move(m_vertexBuffers), program, label);
            }


        private:
            std::vector<std::shared_ptr<IndexBuffer>> m_indexBuffers;

            std::vector<std::shared_ptr<ext::StructuredVertexBuffer>> m_vertexBuffers;
        };
    }
}
