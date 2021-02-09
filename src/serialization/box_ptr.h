#pragma once

#include "engine/world.h"
#include "optional.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::Box* box, const Serializer<engine::World>& ser)
{
  if(box == nullptr)
    return std::nullopt;

  ser.tag("box");
  return gsl::narrow<uint32_t>(std::distance(&ser.context.getBoxes().at(0), box));
}

inline std::optional<uint32_t> ptrSave(loader::file::Box* box, const Serializer<engine::World>& ser)
{
  return ptrSave(const_cast<const loader::file::Box*>(box), ser);
}

inline const loader::file::Box*
  ptrLoad(const TypeId<const loader::file::Box*>&, std::optional<uint32_t> idx, const Serializer<engine::World>& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("box");
  return &ser.context.getBoxes().at(idx.value());
}

inline loader::file::Box*
  ptrLoad(const TypeId<loader::file::Box*>&, std::optional<uint32_t> idx, const Serializer<engine::World>& ser)
{
  return const_cast<loader::file::Box*>(ptrLoad(TypeId<const loader::file::Box*>{}, idx, ser));
}
} // namespace serialization
