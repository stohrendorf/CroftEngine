#pragma once

#include "loader/file/texture.h"
#include "render/gl/structuredvertexbuffer.h"

#include <boost/assert.hpp>
#include <map>
#include <set>
#include <vector>

namespace render
{
class TextureAnimator
{
    struct Sequence
    {
        struct VertexReference
        {
            //! Vertex buffer index
            const size_t bufferIndex;
            const int sourceIndex;
            size_t queueOffset = 0;

            VertexReference(const size_t bufferIdx, const int sourceIdx)
                : bufferIndex(bufferIdx)
                , sourceIndex(sourceIdx)
            {
                Expects(sourceIdx >= 0 && sourceIdx < 4);
            }

            bool operator<(const VertexReference& rhs) const noexcept
            {
                return bufferIndex < rhs.bufferIndex;
            }

            bool operator==(const VertexReference& rhs) const noexcept
            {
                return bufferIndex == rhs.bufferIndex;
            }
        };

        std::vector<core::TextureProxyId> proxyIds;
        std::map<std::shared_ptr<render::gl::StructuredVertexBuffer>, std::set<VertexReference>> affectedVertices;

        void rotate()
        {
            BOOST_ASSERT(!proxyIds.empty());
            auto first = proxyIds.front();
            proxyIds.erase(proxyIds.begin(), std::next(proxyIds.begin()));
            proxyIds.emplace_back(first);
        }

        void registerVertex(const std::shared_ptr<render::gl::StructuredVertexBuffer>& buffer,
                            VertexReference vertex,
                            const core::TextureProxyId proxyId)
        {
            const auto it = std::find(proxyIds.begin(), proxyIds.end(), proxyId);
            Expects(it != proxyIds.end());
            vertex.queueOffset = std::distance(proxyIds.begin(), it);
            affectedVertices[buffer].insert(vertex);
        }

        void updateCoordinates(const std::vector<loader::file::TextureLayoutProxy>& proxies)
        {
            BOOST_ASSERT(!proxyIds.empty());

            for(const auto& partAndVertices : affectedVertices)
            {
                const std::shared_ptr<render::gl::StructuredVertexBuffer>& buffer = partAndVertices.first;
                auto* uvArray = buffer->mapTypedRw<glm::vec2>();

                const std::set<VertexReference>& vertices = partAndVertices.second;

                for(const VertexReference& vref : vertices)
                {
                    BOOST_ASSERT(vref.bufferIndex < buffer->getVertexCount());
                    BOOST_ASSERT(vref.queueOffset < proxyIds.size());
                    const loader::file::TextureLayoutProxy& proxy = proxies[proxyIds[vref.queueOffset].get()];

                    uvArray[vref.bufferIndex] = proxy.uvCoordinates[vref.sourceIndex].toGl();
                }

                render::gl::VertexBuffer::unmap();
            }
        }
    };

    std::vector<Sequence> m_sequences;
    std::map<core::TextureProxyId, size_t> m_sequenceByProxyId;

public:
    explicit TextureAnimator(const std::vector<uint16_t>& data,
                             std::vector<loader::file::TextureLayoutProxy>& textureProxies,
                             std::vector<loader::file::DWordTexture>& textures,
                             bool linear);

    void registerVertex(const core::TextureProxyId proxyId,
                        const std::shared_ptr<render::gl::StructuredVertexBuffer>& buffer,
                        const int sourceIndex,
                        const size_t bufferIndex)
    {
        if(m_sequenceByProxyId.find(proxyId) == m_sequenceByProxyId.end())
            return;

        const size_t sequenceId = m_sequenceByProxyId[proxyId];
        m_sequences.at(sequenceId).registerVertex(buffer, Sequence::VertexReference(bufferIndex, sourceIndex), proxyId);
    }

    void updateCoordinates(const std::vector<loader::file::TextureLayoutProxy>& proxies)
    {
        for(Sequence& sequence : m_sequences)
        {
            sequence.rotate();
            sequence.updateCoordinates(proxies);
        }
    }
};
} // namespace render
