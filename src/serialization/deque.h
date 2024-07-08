#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <deque>
#include <gsl/gsl-lite.hpp>

namespace serialization
{
template<typename T, typename TContext>
void serialize(const std::deque<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("deque");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access<T, false>::dispatch(element, tmp);
  }
}

template<typename T, typename TContext>
void deserialize(std::deque<T>& data, const Deserializer<TContext>& ser)
{
  ser.tag("deque");
  gsl_Expects(ser.node.is_seq());
  data = std::deque<T>();
  std::transform(ser.node.begin(),
                 ser.node.end(),
                 std::back_inserter(data),
                 [&ser](const ryml::ConstNodeRef& element)
                 {
                   return T{access<T, true>::dispatch(ser.withNode(element))};
                 });
}
} // namespace serialization
