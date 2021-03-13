#pragma once

#include "engine/world/world.h"
#include "optional.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const engine::world::Box* box, const Serializer<engine::world::World>& ser)
{
  if(box == nullptr)
    return std::nullopt;

  ser.tag("box");
  return gsl::narrow<uint32_t>(std::distance(&ser.context.getBoxes().at(0), box));
}

inline std::optional<uint32_t> ptrSave(engine::world::Box* box, const Serializer<engine::world::World>& ser)
{
  return ptrSave(const_cast<const engine::world::Box*>(box), ser);
}

inline const engine::world::Box* ptrLoad(const TypeId<const engine::world::Box*>&,
                                         std::optional<uint32_t> idx,
                                         const Serializer<engine::world::World>& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("box");
  return &ser.context.getBoxes().at(idx.value());
}

inline engine::world::Box*
  ptrLoad(const TypeId<engine::world::Box*>&, std::optional<uint32_t> idx, const Serializer<engine::world::World>& ser)
{
  return const_cast<engine::world::Box*>(ptrLoad(TypeId<const engine::world::Box*>{}, idx, ser));
}
} // namespace serialization
