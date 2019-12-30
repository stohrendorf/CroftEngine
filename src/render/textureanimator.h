#pragma once

#include "loader/file/texture.h"
#include "render/gl/vertexbuffer.h"

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

    std::vector<core::TextureTileId> tileIds;
    std::map<std::shared_ptr<gl::VertexBuffer<glm::vec2>>, std::set<VertexReference>> affectedVertices;

    void rotate()
    {
      BOOST_ASSERT(!tileIds.empty());
      auto first = tileIds.front();
      tileIds.erase(tileIds.begin(), std::next(tileIds.begin()));
      tileIds.emplace_back(first);
    }

    void registerVertex(const std::shared_ptr<gl::VertexBuffer<glm::vec2>>& buffer,
                        VertexReference vertex,
                        const core::TextureTileId tileId)
    {
      const auto it = std::find(tileIds.begin(), tileIds.end(), tileId);
      Expects(it != tileIds.end());
      vertex.queueOffset = std::distance(tileIds.begin(), it);
      affectedVertices[buffer].insert(vertex);
    }

    void updateCoordinates(const std::vector<loader::file::TextureTile>& tiles)
    {
      BOOST_ASSERT(!tileIds.empty());

      for(const auto& partAndVertices : affectedVertices)
      {
        const std::shared_ptr<gl::VertexBuffer<glm::vec2>>& buffer = partAndVertices.first;
        auto* uvArray = buffer->map(::gl::BufferAccessARB::ReadWrite);

        const std::set<VertexReference>& vertices = partAndVertices.second;

        for(const VertexReference& vref : vertices)
        {
          BOOST_ASSERT(buffer->size() > 0 && vref.bufferIndex < static_cast<size_t>(buffer->size()));
          BOOST_ASSERT(vref.queueOffset < tileIds.size());
          const loader::file::TextureTile& tile = tiles[tileIds[vref.queueOffset].get()];

          uvArray[vref.bufferIndex] = tile.uvCoordinates[vref.sourceIndex].toGl();
        }

        buffer->unmap();
      }
    }
  };

  std::vector<Sequence> m_sequences;
  std::map<core::TextureTileId, size_t> m_sequenceByTileId;

public:
  explicit TextureAnimator(const std::vector<uint16_t>& data,
                           std::vector<loader::file::TextureTile>& textureTiles,
                           std::vector<loader::file::DWordTexture>& textures,
                           bool linear);

  void registerVertex(const core::TextureTileId tileId,
                      const std::shared_ptr<gl::VertexBuffer<glm::vec2>>& buffer,
                      const int sourceIndex,
                      const size_t bufferIndex)
  {
    if(m_sequenceByTileId.find(tileId) == m_sequenceByTileId.end())
      return;

    const size_t sequenceId = m_sequenceByTileId[tileId];
    m_sequences.at(sequenceId).registerVertex(buffer, Sequence::VertexReference(bufferIndex, sourceIndex), tileId);
  }

  void updateCoordinates(const std::vector<loader::file::TextureTile>& tiles)
  {
    for(Sequence& sequence : m_sequences)
    {
      sequence.rotate();
      sequence.updateCoordinates(tiles);
    }
  }
};
} // namespace render
