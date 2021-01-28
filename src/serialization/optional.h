#pragma once

#include "serialization.h"

#include <optional>

namespace serialization
{
template<typename T, typename TContext>
inline void save(std::optional<T>& optional, const Serializer<TContext>& ser)
{
  if(optional.has_value())
  {
    access::callSerializeOrSave(*optional, ser);
  }
  else
  {
    ser.setNull();
  }
}

template<typename T, typename TContext>
inline void load(std::optional<T>& optional, const Serializer<TContext>& ser)
{
  if(ser.isNull())
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

template<typename T, typename TContext>
inline std::optional<T> create(const TypeId<std::optional<T>>&, const Serializer<TContext>& ser)
{
  if(ser.isNull())
  {
    return std::nullopt;
  }
  else
  {
    return access::callCreate(TypeId<T>{}, ser);
  }
}
} // namespace serialization
