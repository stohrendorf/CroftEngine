#pragma once

#include "access.h"

#include <array>
#include <ryml.hpp>

namespace serialization
{
template<typename T, size_t N, typename TContext>
void serialize(const std::array<T, N>& data, const Serializer<TContext>& ser)
{
  ser.tag("array");
  ser.node |= ryml::SEQ;
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    const auto tmp = ser.newChild();
    access<T, false>::dispatch(data[i], tmp);
  }
}

template<typename T, size_t N, typename TContext>
void deserialize(std::array<T, N>& data, const Deserializer<TContext>& ser)
{
  ser.tag("array");
  Expects(ser.node.is_seq());
  Expects(ser.node.num_children() == N);
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    access<T, true>::dispatch(data[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
