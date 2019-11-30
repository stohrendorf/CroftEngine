#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline core::TypeId::type ptrSave(const loader::file::SkeletalModelType* model, const Serializer& ser)
{
  if(model == nullptr)
    return std::numeric_limits<core::TypeId::type>::max();

  return ser.engine.find(model).get();
}

inline const loader::file::SkeletalModelType*
  ptrLoad(const TypeId<const loader::file::SkeletalModelType*>&, core::TypeId::type idx, const Serializer& ser)
{
  if(idx == std::numeric_limits<core::TypeId::type>::max())
    return nullptr;

  return ser.engine.findAnimatedModelForType(core::TypeId{idx}).get();
}
} // namespace serialization
