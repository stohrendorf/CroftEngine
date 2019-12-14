#pragma once

#include "engine/engine.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::Animation* anim, const Serializer& ser)
{
  if(anim == nullptr)
    return std::nullopt;

  ser.tag("animation");
  return gsl::narrow<uint32_t>(std::distance(&ser.engine.getAnimations().at(0), anim));
}

inline const loader::file::Animation*
  ptrLoad(const TypeId<const loader::file::Animation*>&, std::optional<uint32_t> idx, const Serializer& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("animation");
  return &ser.engine.getAnimations().at(idx.value());
}
} // namespace serialization
