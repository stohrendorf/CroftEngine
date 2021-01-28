#pragma once

#include "serialization_fwd.h"

#include <vector>

namespace serialization
{
template<typename T, typename TContext>
void save(std::vector<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("vector");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access::callSerializeOrSave(element, tmp);
  }
}

template<typename T, typename TContext>
void load(std::vector<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("vector");
  Expects(ser.node.is_seq());
  data = std::vector<T>();
  data.reserve(ser.node.num_children());
  std::transform(ser.node.begin(), ser.node.end(), std::back_inserter(data), [&ser](const ryml::NodeRef& element) {
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

  template<typename TContext>
  void load(const Serializer<TContext>& ser)
  {
    Expects(ser.node.num_children() == vec.size());
    auto it = vec.begin();
    for(const auto& element : ser.node.children())
    {
      access::callSerializeOrLoad(*it++, ser.withNode(element));
    }
  }

  template<typename TContext>
  void save(const Serializer<TContext>& ser) const
  {
    ser.node |= ryml::SEQ;
    for(auto& element : vec)
    {
      const auto tmp = ser.newChild();
      access::callSerializeOrSave(element, tmp);
    }
  }
};
} // namespace serialization
