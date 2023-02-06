#pragma once

#include "core/id.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/soglb_fwd.h>
#include <glm/ext/scalar_int_sized.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace engine::world
{
struct AtlasTile;
}

namespace render
{
struct AnimatedUV
{
  glm::vec3 uv{0, 0, -1};
  glm::vec4 quadUv12{};
  glm::vec4 quadUv34{};

  explicit AnimatedUV() = default;
  explicit AnimatedUV(glm::int32 index, const glm::vec2& uv, const glm::vec4& quadUv12, const glm::vec4& quadUv34)
      : uv{uv, index}
      , quadUv12{quadUv12}
      , quadUv34{quadUv34}
  {
  }
};

class TextureAnimator
{
public:
  explicit TextureAnimator(const std::vector<uint16_t>& data);

  void registerVertex(const core::TextureTileId& tileId, const int sourceIndex, const size_t bufferIndex)
  {
    auto it = m_sequenceByTileId.find(tileId);
    if(it == m_sequenceByTileId.end())
      return;

    m_sequences.at(it->second).registerVertex(Sequence::VertexReference(bufferIndex, sourceIndex), tileId);
  }

  void updateCoordinates(gl::VertexBuffer<AnimatedUV>& buffer, const std::vector<engine::world::AtlasTile>& tiles)
  {
    for(Sequence& sequence : m_sequences)
    {
      sequence.rotate();
      sequence.updateCoordinates(buffer, tiles);
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
        gsl_Expects(sourceIdx >= 0 && sourceIdx < 4);
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
    std::set<VertexReference> affectedVertices;

    void rotate()
    {
      BOOST_ASSERT(!tileIds.empty());
      std::rotate(tileIds.begin(), std::next(tileIds.begin()), tileIds.end());
    }

    void registerVertex(VertexReference vertex, const core::TextureTileId& tileId)
    {
      const auto it = std::find(tileIds.begin(), tileIds.end(), tileId);
      gsl_Assert(it != tileIds.end());
      vertex.queueOffset = std::distance(tileIds.begin(), it);
      affectedVertices.insert(vertex);
    }

    void updateCoordinates(gl::VertexBuffer<AnimatedUV>& buffer, const std::vector<engine::world::AtlasTile>& tiles);
  };

  std::vector<Sequence> m_sequences;
  std::map<core::TextureTileId, size_t> m_sequenceByTileId;
};
} // namespace render
