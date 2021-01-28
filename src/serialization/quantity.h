#pragma once

#include "qs/quantity.h"
#include "serialization_fwd.h"

namespace serialization
{
template<typename U, typename T, typename TContext>
void serialize(qs::quantity<U, T>& data, const Serializer<TContext>& ser)
{
  ser.tag(U::suffix());
  if(ser.loading)
  {
    T tmp{};
    ser.node >> tmp;
    data = qs::quantity<U, T>{tmp};
  }
  else
  {
    ser.node << data.get();
  }
}
} // namespace serialization
