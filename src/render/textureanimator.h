#pragma once

#include "loader/texture.h"

#include "Mesh.h"

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


                VertexReference(size_t bufferIdx, int sourceIdx)
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


            std::vector<uint16_t> proxyIds;
            std::map<std::shared_ptr<gameplay::Mesh>, std::set<VertexReference>> affectedVertices;


            void rotate()
            {
                BOOST_ASSERT(!proxyIds.empty());
                auto first = proxyIds.front();
                proxyIds.erase(proxyIds.begin(), std::next(proxyIds.begin()));
                proxyIds.emplace_back(first);
            }


            void registerVertex(const std::shared_ptr<gameplay::Mesh>& mesh, VertexReference vertex, uint16_t proxyId)
            {
                //! @fixme Expects(mesh->getVertexFormat().getElement(0).usage == gameplay::VertexFormat::TEXCOORD);

                auto it = std::find(proxyIds.begin(), proxyIds.end(), proxyId);
                Expects(it != proxyIds.end());
                vertex.queueOffset = std::distance(proxyIds.begin(), it);
                affectedVertices[mesh].insert(vertex);
            }


            void updateCoordinates(const std::vector<loader::TextureLayoutProxy>& proxies)
            {
                BOOST_ASSERT(!proxyIds.empty());

                for( const auto& partAndVertices : affectedVertices )
                {
                    const std::shared_ptr<gameplay::Mesh>& mesh = partAndVertices.first;
                    BOOST_ASSERT(mesh->getBuffers().size() == 2);

                    auto* uvArray = mesh->getBuffer(1).mapTypedRw<glm::vec2>();

                    const std::set<VertexReference>& vertices = partAndVertices.second;

                    for( const VertexReference& vref : vertices )
                    {
                        BOOST_ASSERT(vref.bufferIndex < mesh->getBuffer(1).getVertexCount());
                        BOOST_ASSERT(vref.queueOffset < proxyIds.size());
                        const loader::TextureLayoutProxy& proxy = proxies[proxyIds[vref.queueOffset]];

                        uvArray[vref.bufferIndex] = proxy.uvCoordinates[vref.sourceIndex].toGl();
                    }

                    mesh->getBuffer(1).unmap();
                }
            }
        };


        std::vector<Sequence> m_sequences;
        std::map<uint16_t, size_t> m_sequenceByProxyId;

    public:
        explicit TextureAnimator(const std::vector<uint16_t>& data)
        {
            /*
             * We have N rotating sequences, each consisting of M+1 proxy ids.
             */

            const uint16_t* ptr = data.data();
            const auto sequenceCount = *ptr++;

            for( size_t i = 0; i < sequenceCount; ++i )
            {
                Sequence sequence;
                const auto n = *ptr++;
                for( size_t j = 0; j <= n; ++j )
                {
                    BOOST_ASSERT(ptr <= &data.back());
                    const auto proxyId = *ptr++;
                    sequence.proxyIds.emplace_back(proxyId);
                    m_sequenceByProxyId.insert(std::make_pair(proxyId, m_sequences.size()));
                }
                m_sequences.emplace_back(std::move(sequence));
            }
        }


        void registerVertex(uint16_t proxyId, const std::shared_ptr<gameplay::Mesh>& mesh, int sourceIndex, size_t bufferIndex)
        {
            if( m_sequenceByProxyId.find(proxyId) == m_sequenceByProxyId.end() )
                return;

            const size_t sequenceId = m_sequenceByProxyId[proxyId];
            Expects(sequenceId < m_sequences.size());
            m_sequences[sequenceId].registerVertex(mesh, Sequence::VertexReference(bufferIndex, sourceIndex), proxyId);
        }


        void updateCoordinates(const std::vector<loader::TextureLayoutProxy>& proxies)
        {
            for( Sequence& sequence : m_sequences )
            {
                sequence.rotate();
                sequence.updateCoordinates(proxies);
            }
        }
    };
}
