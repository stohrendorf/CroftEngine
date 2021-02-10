#pragma once

#include "access.h"

#include <map>
#include <ryml.hpp>

namespace serialization
{
template<typename T, typename U, typename TContext>
void save(std::map<T, U>& data, const Serializer<TContext>& ser)
{
  ser.node |= ryml::SEQ;
  ser.tag("map");
  for(auto& [key, value] : data)
  {
    const auto tmp = ser.newChild();
    access::callSerializeOrSave(const_cast<T&>(key), tmp["key"]);
    access::callSerializeOrSave(value, tmp["value"]);
  }
}

template<typename T, typename U, typename TContext>
void load(std::map<T, U>& data, const Serializer<TContext>& ser)
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
