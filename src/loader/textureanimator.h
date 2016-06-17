#pragma once

#include <vector>
#include <deque>
#include <set>

#include <irrlicht.h>
#include <gsl.h>

#include "datatypes.h"

class TextureAnimator
{
    std::vector<irr::scene::IMeshBuffer*> m_meshBuffers;

    struct Sequence
    {
        struct VertexReference
        {
            const irr::u16 bufferIndex;
            const int sourceIndex;

            VertexReference(irr::u16 bufferIdx, int sourceIdx)
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

        std::deque<uint16_t> proxyIds;
        std::map<irr::scene::IMeshBuffer*, std::set<VertexReference>> affectedVertices;

        void rotate()
        {
            auto first = proxyIds.front();
            proxyIds.pop_front();
            proxyIds.emplace_back(first);
        }

        void registerVertex(irr::scene::IMeshBuffer* buffer, const VertexReference& vertex)
        {
            Expects(buffer != nullptr);
            affectedVertices[buffer].insert(vertex);
        }

        void updateCoordinates(const std::vector<loader::TextureLayoutProxy>& proxies)
        {
            BOOST_ASSERT(!proxyIds.empty());
            Expects(proxyIds.front() < proxies.size());
            const loader::TextureLayoutProxy& proxy = proxies[proxyIds.front()];

            for(const auto& bufferAndVertices : affectedVertices)
            {
                irr::scene::IMeshBuffer* buffer = bufferAndVertices.first;
                const std::set<VertexReference>& vertices = bufferAndVertices.second;
                for(const VertexReference& vref : vertices)
                {
                    BOOST_ASSERT(vref.bufferIndex < buffer->getVertexCount());
                    irr::core::vector2df* uv = nullptr;
                    switch(buffer->getVertexType())
                    {
                        case irr::video::EVT_STANDARD:
                        {
                            irr::video::S3DVertex* vdata = reinterpret_cast<irr::video::S3DVertex*>(buffer->getVertices());
                            uv = &vdata[vref.bufferIndex].TCoords;
                        }
                        break;
                        case irr::video::EVT_2TCOORDS:
                        {
                            irr::video::S3DVertex2TCoords* vdata = reinterpret_cast<irr::video::S3DVertex2TCoords*>(buffer->getVertices());
                            uv = &vdata[vref.bufferIndex].TCoords;
                        }
                        break;
                        case irr::video::EVT_TANGENTS:
                        {
                            irr::video::S3DVertexTangents* vdata = reinterpret_cast<irr::video::S3DVertexTangents*>(buffer->getVertices());
                            uv = &vdata[vref.bufferIndex].TCoords;
                        }
                        break;
                        default:
                            throw std::runtime_error("Unexpected vertex format");
                    }
                    uv->X = proxy.uvCoordinates[vref.sourceIndex].xpixel / 255.0f;
                    uv->Y = proxy.uvCoordinates[vref.sourceIndex].ypixel / 255.0f;
                }
            }
        }
    };

    std::vector<Sequence> m_sequences;
    std::map<uint16_t, size_t> m_sequenceByProxyId;

public:
    explicit TextureAnimator(const std::vector<uint16_t>& data)
    {
        const uint16_t* ptr = data.data();
        const auto sequenceCount = *ptr++;

        for(size_t i = 0; i < sequenceCount; ++i)
        {
            Sequence sequence;
            const auto n = *ptr++;
            for(size_t j = 0; j < n + 1u; ++j)
            {
                const auto proxyId = *ptr++;
                BOOST_ASSERT(ptr <= &data.back() + 1);
                sequence.proxyIds.emplace_back(proxyId);
                m_sequenceByProxyId.insert(std::make_pair(proxyId, m_sequences.size()));
            }
            m_sequences.emplace_back(std::move(sequence));
        }
    }

    void registerVertex(uint16_t proxyId, irr::scene::IMeshBuffer* buffer, int sourceIndex, irr::u16 bufferIndex)
    {
        if(m_sequenceByProxyId.find(proxyId) == m_sequenceByProxyId.end())
            return;

        Expects(buffer != nullptr)
        const size_t sequenceId = m_sequenceByProxyId[proxyId];
        Expects(sequenceId < m_sequences.size());
        m_sequences[sequenceId].registerVertex(buffer, Sequence::VertexReference(bufferIndex, sourceIndex));
    }

    void updateCoordinates(const std::vector<loader::TextureLayoutProxy>& proxies)
    {
        for(Sequence& sequence : m_sequences)
        {
            sequence.rotate();
            sequence.updateCoordinates(proxies);
        }
    }
};
