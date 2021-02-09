#include "textureanimator.h"

#include "loader/file/datatypes.h"
#include "textureatlas.h"

#include <gl/vertexbuffer.h>
#include <memory>

namespace render
{
TextureAnimator::TextureAnimator(const std::vector<uint16_t>& data)
{
  const uint16_t* ptr = data.data();
  const auto sequenceCount = *ptr++;
  if(sequenceCount == 0)
    return;

  for(size_t i = 0; i < sequenceCount; ++i)
  {
    Sequence sequence;
    const auto n = *ptr++;
    for(size_t j = 0; j <= n; ++j)
    {
      Expects(ptr <= &data.back());
      const auto tileId = *ptr++;
      sequence.tileIds.emplace_back(tileId);
      m_sequenceByTileId.emplace(tileId, m_sequences.size());
    }
    m_sequences.emplace_back(std::move(sequence));
  }
}

void TextureAnimator::Sequence::updateCoordinates(const std::vector<loader::file::TextureTile>& tiles)
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
} // namespace render