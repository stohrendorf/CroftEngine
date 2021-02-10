#pragma once

#include "access.h"

#include <ryml.hpp>
#include <unordered_set>

namespace serialization
{
template<typename T, typename TContext>
void save(std::unordered_set<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("set");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access<T>::callSerializeOrSave(element, tmp);
  }
}

template<typename T, typename TContext>
void load(std::unordered_set<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("set");
  data = std::unordered_set<T>();
  data.reserve(ser.node.num_children());
  for(const auto& element : ser.node.children())
  {
    data.emplace(access<T>::callCreate(ser.withNode(element)));
  }
}
} // namespace serialization
