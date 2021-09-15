#include "floordata.h"

#include "serialization/bitset.h"
#include "serialization/serialization.h"
#include "types.h"

#include <exception>

namespace engine::floordata
{
void ActivationState::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("oneshot", m_oneshot),
      S_NV("inverted", m_inverted),
      S_NV("locked", m_locked),
      S_NV("activationSet", m_activationSet));
}

ActivationState ActivationState::create(const serialization::Serializer<world::World>& ser)
{
  ActivationState result{};
  result.serialize(ser);
  return result;
}

std::optional<uint8_t> getBoundaryRoom(const FloorDataValue* fdData)
{
  if(fdData == nullptr)
    return {};

  FloorDataChunk chunk{fdData[0]};
  if(chunk.type == FloorDataChunkType::FloorSlant)
  {
    if(chunk.isLast)
      return {};
    fdData += 2;
    chunk = FloorDataChunk{fdData[0]};
  }
  if(chunk.type == FloorDataChunkType::CeilingSlant)
  {
    if(chunk.isLast)
      return {};
    fdData += 2;
    chunk = FloorDataChunk{fdData[0]};
  }
  if(chunk.type == FloorDataChunkType::BoundaryRoom)
  {
    return gsl::narrow_cast<uint8_t>(fdData[1].get());
  }

  return {};
}
} // namespace engine::floordata
