#pragma once

#include "access.h"

#include <map>
#include <ryml.hpp>

namespace serialization
{
template<typename T, typename U, typename TContext>
void serialize(std::pair<T, U>& data, const Serializer<TContext>& ser)
{
  ser.tag("pair");
  ser(S_NV("first", data.first), S_NV("second", data.second));
}

template<typename T, typename U, typename TContext>
inline std::pair<T, U> create(const TypeId<std::pair<T, U>>&, const Serializer<TContext>& ser)
{
  ser.tag("pair");
  Expects(ser.loading);
  return {access<T>::callCreate(ser.withNode(ser.node["first"])),
          access<U>::callCreate(ser.withNode(ser.node["second"]))};
}
} // namespace serialization
