#pragma once

#include "engine/engine.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
inline std::optional<uint32_t> ptrSave(const loader::file::AnimFrame* frame, const Serializer& ser)
{
  if(frame == nullptr)
    return std::nullopt;

  ser.tag("animframe");
  return gsl::narrow<uint32_t>(
    std::distance(&ser.engine.getPoseFrames().at(0), reinterpret_cast<const int16_t*>(frame)));
}

inline const loader::file::AnimFrame*
  ptrLoad(const TypeId<const loader::file::AnimFrame*>&, std::optional<uint32_t> idx, const Serializer& ser)
{
  if(!idx.has_value())
    return nullptr;

  ser.tag("animframe");
  return reinterpret_cast<const loader::file::AnimFrame*>(&ser.engine.getPoseFrames().at(idx.value()));
}
} // namespace serialization
