#pragma once

#include "serialization_fwd.h"

#include <deque>

namespace serialization
{
template<typename T>
void save(const std::deque<T>& data, const Serializer& ser)
{
  ser.tag("deque");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access::callSerializeOrSave(element, tmp);
  }
}

template<typename T>
void load(std::deque<T>& data, const Serializer& ser)
{
  ser.tag("deque");
  data = std::deque<T>();
  std::transform(ser.node.begin(), ser.node.end(), std::back_inserter(data), [&ser](const ryml::NodeRef& element) {
    return T{access::callCreate(TypeId<T>{}, ser.withNode(element))};
  });
}
} // namespace serialization
