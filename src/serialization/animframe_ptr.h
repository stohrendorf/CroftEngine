#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline uint32_t ptrSave(const loader::file::AnimFrame* frame, const Serializer& ser)
{
  ser.tag("animframe");
  if(frame == nullptr)
    return std::numeric_limits<uint32_t>::max();

  return gsl::narrow<uint32_t>(
    std::distance(&ser.engine.getPoseFrames().at(0), reinterpret_cast<const int16_t*>(frame)));
}

inline const loader::file::AnimFrame*
  ptrLoad(const TypeId<const loader::file::AnimFrame*>&, uint32_t idx, const Serializer& ser)
{
  ser.tag("animframe");
  if(idx == std::numeric_limits<uint32_t>::max())
    return nullptr;

  return reinterpret_cast<const loader::file::AnimFrame*>(&ser.engine.getPoseFrames().at(idx));
}
} // namespace serialization
