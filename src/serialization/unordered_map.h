#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <gsl/gsl-lite.hpp>
#include <unordered_map>

namespace serialization
{
template<typename T, typename U, typename TContext>
void serialize(const std::unordered_map<T, U>& data, const Serializer<TContext>& ser)
{
  ser.tag("map");
  ser.node |= ryml::SEQ;
  for(auto& [key, value] : data)
  {
    const auto tmp = ser.newChild();
    tmp(S_NV("key", key), S_NV("value", value));
  }
}

template<typename T, typename U, typename TContext>
void deserialize(std::unordered_map<T, U>& data, const Deserializer<TContext>& ser)
{
  ser.tag("map");
  data = std::unordered_map<T, U>();
  for(const auto& element : ser.node.children())
  {
    gsl_Assert(element.is_map());
    gsl_Assert(element.num_children() == 2);
    auto elemSer = ser.withNode(element);
    data.emplace(access<T, true>::dispatch(elemSer["key"]), access<U, true>::dispatch(elemSer["value"]));
  }
}
} // namespace serialization
