#pragma once

#include "engine/world/world.h"
#include "optional.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::TypedBox* box, const Serializer<engine::world::World>& ser)
{
  if(box == nullptr)
    return std::nullopt;

  ser.tag("box");
  return gsl::narrow<uint32_t>(std::distance(&ser.context.getBoxes().at(0), box));
}

inline std::optional<uint32_t> ptrSave(loader::file::TypedBox* box, const Serializer<engine::world::World>& ser)
{
  return ptrSave(const_cast<const loader::file::TypedBox*>(box), ser);
}

inline const loader::file::TypedBox* ptrLoad(const TypeId<const loader::file::TypedBox*>&,
                                             std::optional<uint32_t> idx,
                                             const Serializer<engine::world::World>& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("box");
  return &ser.context.getBoxes().at(idx.value());
}

inline loader::file::TypedBox* ptrLoad(const TypeId<loader::file::TypedBox*>&,
                                       std::optional<uint32_t> idx,
                                       const Serializer<engine::world::World>& ser)
{
  return const_cast<loader::file::TypedBox*>(ptrLoad(TypeId<const loader::file::TypedBox*>{}, idx, ser));
}
} // namespace serialization
