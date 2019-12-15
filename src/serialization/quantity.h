#pragma once

#include "qs/quantity.h"
#include "serialization_fwd.h"

namespace serialization
{
template<typename U, typename T>
void serialize(qs::quantity<U, T>& data, const Serializer& ser)
{
  ser.tag(U::suffix());
  if(ser.loading)
  {
    data = qs::quantity<U, T>{ser.node.as<T>()};
  }
  else
  {
    ser.node = data.get();
  }
}
} // namespace serialization
