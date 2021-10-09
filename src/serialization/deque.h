#pragma once

#include "access.h"

#include <deque>
#include <ryml.hpp>

namespace serialization
{
template<typename T, typename TContext>
void save(std::deque<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("deque");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access<T>::callSerializeOrSave(element, tmp);
  }
}

template<typename T, typename TContext>
void load(std::deque<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("deque");
  Expects(ser.node.is_seq());
  data = std::deque<T>();
  std::transform(ser.node.begin(),
                 ser.node.end(),
                 std::back_inserter(data),
                 [&ser](const ryml::NodeRef& element)
                 {
                   return T{access<T>::callCreate(ser.withNode(element))};
                 });
}
} // namespace serialization
