#pragma once

#include "serialization_fwd.h"

#include <unordered_set>

namespace serialization
{
template<typename T>
void save(std::unordered_set<T>& data, const Serializer& ser)
{
  ser.tag("set");
  ser.node = YAML::Node(YAML::NodeType::Sequence);
  for(auto& element : data)
  {
    const auto tmp = ser.withNode(YAML::Node{});
    access::callSerializeOrSave(element, tmp);
    ser.node.push_back(tmp.node);
  }
}

template<typename T>
void load(std::unordered_set<T>& data, const Serializer& ser)
{
  ser.tag("set");
  data = std::unordered_set<T>();
  data.reserve(ser.node.size());
  for(const auto& element : ser.node)
  {
    data.emplace(access::callCreate(TypeId<T>{}, ser.withNode(element)));
  }
}
} // namespace serialization
