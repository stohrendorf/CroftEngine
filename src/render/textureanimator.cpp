#include "textureanimator.h"

#include "engine/world/atlastile.h"
#include "loader/file/datatypes.h"
#include "loader/file/texture.h"

#include <array>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/vertexbuffer.h>
#include <gsl/gsl-lite.hpp>
#include <utility>
#include <vector>

namespace render
{
TextureAnimator::TextureAnimator(const std::vector<uint16_t>& data)
{
  gsl_Expects(!data.empty());

  const uint16_t* ptr = data.data();
  const auto sequenceCount = *ptr++;
  for(size_t i = 0; i < sequenceCount; ++i)
  {
    Sequence sequence;
    const auto n = *ptr++;
    sequence.tileIds.reserve(n + 1);
    for(size_t j = 0; j <= n; ++j)
    {
      gsl_Assert(ptr <= &data.back());
      const auto tileId = *ptr++;
      sequence.tileIds.emplace_back(tileId);
      m_sequenceByTileId.emplace(tileId, m_sequences.size());
    }
    m_sequences.emplace_back(std::move(sequence));
  }

  BOOST_ASSERT(ptr <= &data.back() + 1);
}

void TextureAnimator::Sequence::updateCoordinates(gl::VertexBuffer<AnimatedUV>& buffer,
                                                  const std::vector<engine::world::AtlasTile>& tiles)
{
  BOOST_ASSERT(!tileIds.empty());

  auto uvArray
    = buffer.map(gl::api::MapBufferAccessMask::MapWriteBit | gl::api::MapBufferAccessMask::MapFlushExplicitBit);

  for(const VertexReference& vref : affectedVertices)
  {
    BOOST_ASSERT(buffer.size() > 0 && vref.getBufferIndex() < static_cast<size_t>(buffer.size()));
    BOOST_ASSERT(vref.queueOffset < tileIds.size());
    const engine::world::AtlasTile& tile = tiles[tileIds[vref.queueOffset].get()];

    uvArray[vref.getBufferIndex()].uv = glm::vec3{tile.uvCoordinates[vref.getSourceIndex()],
                                                  tile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask};
    uvArray[vref.getBufferIndex()].quadUv12 = glm::vec4{tile.uvCoordinates[0], tile.uvCoordinates[1]};
    uvArray[vref.getBufferIndex()].quadUv34 = glm::vec4{tile.uvCoordinates[2], tile.uvCoordinates[3]};
  }

  uvArray.flush();
}
} // namespace render
