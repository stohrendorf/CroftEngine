#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <optional>

namespace serialization
{
template<typename T, typename TContext>
inline void serialize(const std::optional<T>& optional, const Serializer<TContext>& ser)
{
  if(optional.has_value())
  {
    access<T, false>::dispatch(*optional, ser);
  }
  else
  {
    ser.setNull();
  }
}

template<typename T, typename TContext>
inline void deserialize(std::optional<T>& optional, const Deserializer<TContext>& ser)
{
  if(ser.isNull())
  {
    optional.reset();
  }
  else
  {
    if(optional.has_value())
      access<T, true>::dispatch(*optional, ser);
    else
      optional = access<T, true>::dispatch(ser);
  }
}

template<typename T, typename TContext>
inline std::optional<T> create(const TypeId<std::optional<T>>&, const Deserializer<TContext>& ser)
{
  if(ser.isNull())
  {
    return std::nullopt;
  }
  else
  {
    return access<T, true>::dispatch(ser);
  }
}
} // namespace serialization
