#pragma once

#include "serialization_fwd.h"

#include <deque>

namespace serialization
{
template<typename T>
void save(std::deque<T>& data, const Serializer& ser)
{
  ser.tag("deque");
  ser.node = YAML::Node(YAML::NodeType::Sequence);
  for(auto& element : data)
  {
    const auto tmp = ser.withNode(YAML::Node{});
    access::callSerializeOrSave(element, tmp);
    ser.node.push_back(tmp.node);
  }
}

template<typename T>
void load(std::deque<T>& data, const Serializer& ser)
{
  ser.tag("deque");
  data = std::deque<T>();
  for(const auto& element : ser.node)
  {
    data.emplace_back(access::callCreate(TypeId<T>{}, ser.withNode(element)));
  }
}
} // namespace serialization
