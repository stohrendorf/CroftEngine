#include "floordata.h"

#include "serialization/bitset.h"
#include "serialization/serialization.h"
#include "types.h"
#include "util/helpers.h"

#include <exception>
#include <type_traits>

namespace engine::floordata
{
void ActivationState::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("oneshot", m_oneshot),
      S_NV("inverted", m_inverted),
      S_NV("locked", m_locked),
      S_NV("activationSet", m_activationSet));
}

void ActivationState::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("oneshot", m_oneshot),
      S_NV("inverted", m_inverted),
      S_NV("locked", m_locked),
      S_NV("activationSet", m_activationSet));
}

ActivationState ActivationState::create(const serialization::Deserializer<world::World>& ser)
{
  ActivationState result{};
  result.deserialize(ser);
  return result;
}

ActivationState::ActivationSet ActivationState::extractActivationSet(const FloorDataValue& fd)
{
  const auto bits = gsl::narrow_cast<uint16_t>(util::bits(fd.get(), 9, 5));
  return ActivationSet{bits};
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

CameraParameters::CameraParameters(const FloorDataValue& fd)
    : timeout{core::Seconds{static_cast<core::Seconds::type>(int8_t(fd.get()))}}
    , oneshot{(fd.get() & 0x100u) != 0}
    , isLast{(fd.get() & 0x8000u) != 0}
    , smoothness{gsl::narrow_cast<uint8_t>(util::bits(fd.get(), 9, 5) * 2)}
{
}

CommandOpcode Command::extractOpcode(const FloorDataValue& data)
{
  return gsl::narrow_cast<CommandOpcode>(util::bits(data.get(), 10, 4));
}
} // namespace engine::floordata
