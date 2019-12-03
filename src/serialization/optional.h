#pragma once

#include "serialization.h"

#include <optional>

namespace serialization
{
template<typename T>
inline void save(std::optional<T>& optional, const Serializer& ser)
{
  ser.tag("optional");
  if(optional.has_value())
  {
    ser(S_NV("value", *optional));
  }
  else
  {
    ser.node["value"] = YAML::Node{YAML::NodeType::Null};
  }
}

template<typename T>
inline void load(std::optional<T>& optional, const Serializer& ser)
{
  ser.tag("optional");
  if(ser.node["value"].IsNull())
  {
    optional.reset();
  }
  else
  {
    if(optional.has_value())
      ser(S_NV("value", *optional));
    else
      *optional = access::callCreate(TypeId<T>{}, ser["value"]);
  }
}
} // namespace serialization
