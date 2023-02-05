#pragma once

#include "qs/quantity.h"
#include "serialization_fwd.h"

namespace serialization
{
template<typename U, typename T, typename TContext>
void serialize(const qs::quantity<U, T>& data, const Serializer<TContext>& ser)
{
  ser.tag(U::suffix());
  ser.node << data.get();
}

template<typename U, typename T, typename TContext>
void deserialize(qs::quantity<U, T>& data, const Deserializer<TContext>& ser)
{
  ser.tag(U::suffix());
  T tmp{};
  ser.node >> tmp;
  data = qs::quantity<U, T>{tmp};
}
} // namespace serialization
