#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline core::TypeId::type ptrSave(const loader::file::Sprite* sprite, const Serializer& ser)
{
  if(sprite == nullptr)
    return std::numeric_limits<core::TypeId::type>::max();

  return ser.engine.find(sprite).get();
}

inline const loader::file::Sprite*
  ptrLoad(const TypeId<const loader::file::Sprite*>&, core::TypeId::type idx, const Serializer& ser)
{
  if(idx == std::numeric_limits<core::TypeId::type>::max())
    return nullptr;

  return &ser.engine.findSpriteSequenceForType(core::TypeId{idx})->sprites.at(0);
}
} // namespace serialization
