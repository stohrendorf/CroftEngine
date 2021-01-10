#pragma once

#include "loader/file/datatypes.h"
#include "loader/file/texture.h"

#include <boost/assert.hpp>
#include <gl/vertexbuffer.h>
#include <map>
#include <set>
#include <vector>

namespace gl
{
class CImgWrapper;
}

namespace render
{
class TextureAnimator
{
public:
  struct AnimatedUV
  {
    glm::int32_t index{-1};
    glm::vec2 uv{};

    explicit AnimatedUV() = default;

    explicit AnimatedUV(glm::int32_t index, const glm::vec2& uv)
        : index{index}
        , uv{uv}
    {
    }
  };

  explicit TextureAnimator(const std::vector<uint16_t>& data);

  void registerVertex(const core::TextureTileId tileId,
                      const std::shared_ptr<gl::VertexBuffer<AnimatedUV>>& buffer,
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

private:
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
    std::map<std::shared_ptr<gl::VertexBuffer<AnimatedUV>>, std::set<VertexReference>> affectedVertices;

    void rotate()
    {
      BOOST_ASSERT(!tileIds.empty());
      auto first = tileIds.front();
      tileIds.erase(tileIds.begin(), std::next(tileIds.begin()));
      tileIds.emplace_back(first);
    }

    void registerVertex(const std::shared_ptr<gl::VertexBuffer<AnimatedUV>>& buffer,
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

      for(const auto& [buffer, vertices] : affectedVertices)
      {
        auto* uvArray = buffer->map(gl::api::BufferAccessARB::ReadWrite);

        for(const VertexReference& vref : vertices)
        {
          BOOST_ASSERT(buffer->size() > 0 && vref.bufferIndex < static_cast<size_t>(buffer->size()));
          BOOST_ASSERT(vref.queueOffset < tileIds.size());
          const loader::file::TextureTile& tile = tiles[tileIds[vref.queueOffset].get()];

          uvArray[vref.bufferIndex].uv = tile.uvCoordinates[vref.sourceIndex].toGl();
          uvArray[vref.bufferIndex].index = tile.textureKey.tileAndFlag & loader::file::TextureIndexMask;
        }

        buffer->unmap();
      }
    }
  };

  std::vector<Sequence> m_sequences;
  std::map<core::TextureTileId, size_t> m_sequenceByTileId;
};
} // namespace render
