#pragma once

#include "engine/world.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::Room* room, const Serializer& ser)
{
  if(room == nullptr)
    return std::nullopt;

  ser.tag("room");
  return gsl::narrow<uint32_t>(std::distance(const_cast<const loader::file::Room*>(&ser.world.getRooms().at(0)), room));
}

inline const loader::file::Room*
  ptrLoad(const TypeId<const loader::file::Room*>&, std::optional<uint32_t> idx, const Serializer& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("room");
  return &ser.world.getRooms().at(idx.value());
}
} // namespace serialization
