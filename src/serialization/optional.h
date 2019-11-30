#pragma once

#include "serialization.h"

#include <optional>

namespace serialization
{
template<typename T>
inline void save(std::optional<T>& optional, const Serializer& ser)
{
  if(optional.has_value())
  {
    ser("value", *optional);
  }
  else
  {
    ser.node["value"] = YAML::Node();
  }
}

template<typename T>
inline void load(std::optional<T>& optional, const Serializer& ser)
{
  if(ser.node["value"].IsNull())
  {
    optional.reset();
  }
  else
  {
    if(optional.has_value())
      ser("value", *optional);
    else
      *optional = access::callCreate(TypeId<T>{}, ser["value"]);
  }
}
} // namespace serialization
