#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <unordered_set>

namespace serialization
{
template<typename T, typename TContext>
void serialize(std::unordered_set<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("set");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access<T, false>::dispatch(element, tmp);
  }
}

template<typename T, typename TContext>
void deserialize(std::unordered_set<T>& data, const Deserializer<TContext>& ser)
{
  ser.tag("set");
  data = std::unordered_set<T>();
  data.reserve(ser.node.num_children());
  for(const auto& element : ser.node.children())
  {
    data.emplace(access<T, true>::dispatch(ser.withNode(element)));
  }
}
} // namespace serialization
