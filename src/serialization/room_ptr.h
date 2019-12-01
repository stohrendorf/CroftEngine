#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline uint32_t ptrSave(const loader::file::Room* room, const Serializer& ser)
{
  ser.tag("room");
  if(room == nullptr)
    return std::numeric_limits<uint32_t>::max();

  return gsl::narrow<uint32_t>(std::distance(&ser.engine.getRooms().at(0), room));
}

inline const loader::file::Room* ptrLoad(const TypeId<const loader::file::Room*>&, uint32_t idx, const Serializer& ser)
{
  ser.tag("room");
  if(idx == std::numeric_limits<uint32_t>::max())
    return nullptr;

  return &ser.engine.getRooms().at(idx);
}
} // namespace serialization
