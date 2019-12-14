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
    access::callSerializeOrSave(*optional, ser);
  }
  else
  {
    ser.node = YAML::Node{YAML::NodeType::Null};
  }
}

template<typename T>
inline void load(std::optional<T>& optional, const Serializer& ser)
{
  if(ser.node.IsNull())
  {
    optional.reset();
  }
  else
  {
    if(optional.has_value())
      access::callSerializeOrLoad(*optional, ser);
    else
      *optional = access::callCreate(TypeId<T>{}, ser);
  }
}

template<typename T>
inline std::optional<T> create(const TypeId<std::optional<T>>&, const Serializer& ser)
{
  if(ser.node.IsNull())
  {
    return std::nullopt;
  }
  else
  {
    return access::callCreate(TypeId<T>{}, ser);
  }
}
} // namespace serialization
