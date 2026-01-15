#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <optional>

namespace serialization
{
template<typename T, typename TContext>
void serialize(const std::optional<T>& optional, const Serializer<TContext>& ser)
{
  if(optional.has_value())
  {
    access::dispatchSerialize(*optional, ser);
  }
  else
  {
    ser.setNull();
  }
}

template<typename T, typename TContext>
void deserialize(std::optional<T>& optional, const Deserializer<TContext>& ser)
{
  if(ser.isNull())
  {
    optional.reset();
  }
  else
  {
    if(optional.has_value())
      access::dispatchDeserialize(*optional, ser);
    else
      optional = access::dispatchCreate<T>(ser);
  }
}

template<typename T, typename TContext>
std::optional<T> create(const TypeId<std::optional<T>>&, const Deserializer<TContext>& ser)
{
  if(ser.isNull())
  {
    return std::nullopt;
  }
  else
  {
    return access::dispatchCreate<T>(ser);
  }
}
} // namespace serialization