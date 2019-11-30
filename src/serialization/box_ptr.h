#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
inline uint32_t ptrSave(const loader::file::Box* box, const Serializer& ser)
{
  if(box == nullptr)
    return std::numeric_limits<uint32_t>::max();

  return gsl::narrow<uint32_t>(std::distance(&ser.engine.getBoxes().at(0), box));
}

inline const loader::file::Box* ptrLoad(const TypeId<const loader::file::Box*>&, uint32_t idx, const Serializer& ser)
{
  if(idx == std::numeric_limits<uint32_t>::max())
    return nullptr;

  return &ser.engine.getBoxes().at(idx);
}
} // namespace serialization
