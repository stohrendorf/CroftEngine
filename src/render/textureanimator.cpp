#include "textureanimator.h"

#include "loader/file/datatypes.h"
#include "textureatlas.h"

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
} // namespace render