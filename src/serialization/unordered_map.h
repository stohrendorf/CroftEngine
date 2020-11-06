#pragma once

#include "serialization_fwd.h"

#include <unordered_map>

namespace serialization
{
template<typename T, typename U>
void save(std::unordered_map<T, U>& data, const Serializer& ser)
{
  ser.tag("map");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    tmp(S_NV("key", const_cast<T&>(element.first)), S_NV("value", element.second));
  }
}

template<typename T, typename U>
void load(std::unordered_map<T, U>& data, const Serializer& ser)
{
  ser.tag("map");
  data = std::unordered_map<T, U>();
  for(const auto& element : ser.node.children())
  {
    Expects(element.is_map());
    Expects(element.num_children() == 2);
    auto elemSer = ser.withNode(element);
    data.emplace(access::callCreate(TypeId<T>{}, elemSer["key"]), access::callCreate(TypeId<U>{}, elemSer["value"]));
  }
}
} // namespace serialization
