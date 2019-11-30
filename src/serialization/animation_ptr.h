#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline uint32_t ptrSave(const loader::file::Animation* anim, const Serializer& ser)
{
  if(anim == nullptr)
    return std::numeric_limits<uint32_t>::max();

  return gsl::narrow<uint32_t>(std::distance(&ser.engine.getAnimations().at(0), anim));
}

inline const loader::file::Animation*
  ptrLoad(const TypeId<const loader::file::Animation*>&, uint32_t idx, const Serializer& ser)
{
  if(idx == std::numeric_limits<uint32_t>::max())
    return nullptr;

  return &ser.engine.getAnimations().at(idx);
}
} // namespace serialization
