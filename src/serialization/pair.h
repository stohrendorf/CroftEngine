#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <utility>

namespace serialization
{
template<typename T, typename U, typename TContext>
void serialize(const std::pair<T, U>& data, const Serializer<TContext>& ser)
{
  ser.tag("pair");
  ser(S_NV("first", data.first), S_NV("second", data.second));
}

template<typename T, typename U, typename TContext>
inline std::pair<T, U> create(const TypeId<std::pair<T, U>>&, const Deserializer<TContext>& ser)
{
  ser.tag("pair");
  return {access<T, true>::dispatch(ser.withNode(ser.node["first"])),
          access<U, true>::dispatch(ser.withNode(ser.node["second"]))};
}
} // namespace serialization
