#pragma once

#include "engine/world.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
inline std::optional<core::TypeId::type> ptrSave(const loader::file::Sprite* sprite, const Serializer<engine::World>& ser)
{
  if(sprite == nullptr)
    return std::nullopt;

  ser.tag("sprite");
  return ser.context.find(sprite).get();
}

inline const loader::file::Sprite*
  ptrLoad(const TypeId<const loader::file::Sprite*>&, std::optional<core::TypeId::type> idx, const Serializer<engine::World>& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("sprite");
  return &ser.context.findSpriteSequenceForType(core::TypeId{idx.value()})->sprites[0];
}
} // namespace serialization
