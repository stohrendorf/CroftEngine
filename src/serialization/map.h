#pragma once

#include "access.h"

#include <map>
#include <ryml.hpp>

namespace serialization
{
template<typename T, typename U, typename TContext>
void serialize(const std::map<T, U>& data, const Serializer<TContext>& ser)
{
  ser.node |= ryml::SEQ;
  ser.tag("map");
  for(auto& [key, value] : data)
  {
    const auto tmp = ser.newChild();
    access<T, false>::dispatch(key, tmp["key"]);
    access<U, false>::dispatch(value, tmp["value"]);
  }
}

template<typename T, typename U, typename TContext>
void deserialize(std::map<T, U>& data, const Deserializer<TContext>& ser)
{
  ser.tag("map");
  data = std::map<T, U>();
  for(const auto& element : ser.node.children())
  {
    gsl_Assert(element.is_map());
    gsl_Assert(element.num_children() == 2);
    gsl_Assert(element["key"].valid() && element["value"].valid());

    data.emplace(access<T, true>::dispatch(ser.withNode(element["key"])),
                 access<U, true>::dispatch(ser.withNode(element["value"])));
  }
}
} // namespace serialization
