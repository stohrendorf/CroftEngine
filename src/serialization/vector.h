#pragma once

#include "serialization_fwd.h"

#include <vector>

namespace serialization
{
template<typename T>
void save(std::vector<T>& data, const Serializer& ser)
{
  ser.tag("vector");
  ser.node = YAML::Node(YAML::NodeType::Sequence);
  for(auto& element : data)
  {
    const auto tmp = ser.withNode(YAML::Node{});
    access::callSerializeOrSave(element, tmp);
    ser.node.push_back(tmp.node);
  }
}

template<typename T>
void load(std::vector<T>& data, const Serializer& ser)
{
  ser.tag("vector");
  Expects(ser.node.IsSequence());
  data = std::vector<T>();
  data.reserve(ser.node.size());
  std::transform(ser.node.begin(), ser.node.end(), std::back_inserter(data), [&ser](const YAML::Node& element) {
    return access::callCreate(TypeId<T>{}, ser.withNode(element));
  });
}

template<typename T>
struct FrozenVector
{
  std::vector<T>& vec;
  explicit FrozenVector(std::vector<T>& vec)
      : vec{vec}
  {
  }

  void load(const Serializer& ser)
  {
    Expects(ser.node.size() == vec.size());
    auto it = vec.begin();
    for(const auto& element : ser.node)
    {
      access::callSerializeOrLoad(*it++, ser.withNode(element));
    }
  }

  void save(const Serializer& ser)
  {
    ser.node = YAML::Node{YAML::NodeType::Sequence};
    for(auto& element : vec)
    {
      const auto tmp = ser.withNode(YAML::Node{});
      access::callSerializeOrSave(element, tmp);
      ser.node.push_back(tmp.node);
    }
  }
};
} // namespace serialization
