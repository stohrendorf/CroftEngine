#pragma once

#include "serialization_fwd.h"

#include <map>

namespace serialization
{
template<typename T, typename U>
void save(std::map<T, U>& data, const Serializer& ser)
{
  ser.node = YAML::Node(YAML::NodeType::Sequence);
  for(auto& element : data)
  {
    const auto tmp = ser.withNode(YAML::Node{});
    access::callSerializeOrSave(const_cast<T&>(element.first), tmp["key"]);
    access::callSerializeOrSave(element.second, tmp["value"]);
    ser.node.push_back(tmp.node);
  }
}

template<typename T, typename U>
void load(std::map<T, U>& data, const Serializer& ser)
{
  data = std::map<T, U>();
  for(auto& element : ser.node)
  {
    Expects(element.IsMap());
    Expects(element.size() == 2);
    Expects(element["key"].IsDefined() && element["value"].IsDefined());

    data.emplace(access::callCreate(TypeId<T>{}, ser.withNode(element["key"])),
                 access::callCreate(TypeId<U>{}, ser.withNode(element["value"])));
  }
}
} // namespace serialization
