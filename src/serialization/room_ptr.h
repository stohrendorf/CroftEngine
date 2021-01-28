#pragma once

#include "engine/world.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::Room* room, const Serializer<engine::World>& ser)
{
  if(room == nullptr)
    return std::nullopt;

  ser.tag("room");
  return gsl::narrow<uint32_t>(
    std::distance(const_cast<const loader::file::Room*>(&ser.context.getRooms().at(0)), room));
}

inline const loader::file::Room*
  ptrLoad(const TypeId<const loader::file::Room*>&, std::optional<uint32_t> idx, const Serializer<engine::World>& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("room");
  return &ser.context.getRooms().at(idx.value());
}
} // namespace serialization
