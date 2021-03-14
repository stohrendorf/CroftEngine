#pragma once

#include "engine/world/world.h"
#include "ptr.h"

namespace serialization
{
inline core::TypeId::type ptrSave(const engine::world::SkeletalModelType* model,
                                  const Serializer<engine::world::World>& ser)
{
  ser.tag("skeletalmodel");
  if(model == nullptr)
    return std::numeric_limits<core::TypeId::type>::max();

  return ser.context.find(model).get();
}

inline const engine::world::SkeletalModelType* ptrLoad(const TypeId<const engine::world::SkeletalModelType*>&,
                                                       core::TypeId::type idx,
                                                       const Serializer<engine::world::World>& ser)
{
  ser.tag("skeletalmodel");
  if(idx == std::numeric_limits<core::TypeId::type>::max())
    return nullptr;

  return ser.context.findAnimatedModelForType(core::TypeId{idx}).get();
}
} // namespace serialization
