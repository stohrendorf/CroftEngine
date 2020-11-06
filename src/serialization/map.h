#pragma once

#include "serialization_fwd.h"

#include <map>

namespace serialization
{
template<typename T, typename U>
void save(std::map<T, U>& data, const Serializer& ser)
{
  ser.node |= ryml::SEQ;
  ser.tag("map");
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access::callSerializeOrSave(const_cast<T&>(element.first), tmp["key"]);
    access::callSerializeOrSave(element.second, tmp["value"]);
  }
}

template<typename T, typename U>
void load(std::map<T, U>& data, const Serializer& ser)
{
  ser.tag("map");
  data = std::map<T, U>();
  for(const auto& element : ser.node.children())
  {
    Expects(element.is_map());
    Expects(element.num_children() == 2);
    Expects(element["key"].valid() && element["value"].valid());

    data.emplace(access::callCreate(TypeId<T>{}, ser.withNode(element["key"])),
                 access::callCreate(TypeId<U>{}, ser.withNode(element["value"])));
  }
}
} // namespace serialization
