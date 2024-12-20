#pragma once

#include "core/id.h"
#include "engine/world/world.h"
#include "engine/world/worldgeometry.h"
#include "ptr.h" // IWYU pragma: keep
#include "serialization.h"

#include <limits>

namespace serialization
{
inline core::TypeId::type ptrSave(const engine::world::SkeletalModelType* const& model,
                                  const Serializer<engine::world::World>& ser)
{
  ser.tag("skeletalmodel");
  if(model == nullptr)
    return std::numeric_limits<core::TypeId::type>::max();

  return ser.context->getWorldGeometry().find(model).get();
}

inline const engine::world::SkeletalModelType* ptrLoad(const TypeId<const engine::world::SkeletalModelType*>&,
                                                       core::TypeId::type idx,
                                                       const Deserializer<engine::world::World>& ser)
{
  ser.tag("skeletalmodel");
  if(idx == std::numeric_limits<core::TypeId::type>::max())
    return nullptr;

  return ser.context->getWorldGeometry().findAnimatedModelForType(core::TypeId{idx}).get();
}
} // namespace serialization
