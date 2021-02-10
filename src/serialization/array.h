#pragma once

#include "access.h"

#include <array>
#include <ryml.hpp>

namespace serialization
{
template<typename T, size_t N, typename TContext>
void save(std::array<T, N>& data, const Serializer<TContext>& ser)
{
  ser.tag("array");
  ser.node |= ryml::SEQ;
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    const auto tmp = ser.newChild();
    access<T>::callSerializeOrSave(data[i], tmp);
  }
}

template<typename T, size_t N, typename TContext>
void load(std::array<T, N>& data, const Serializer<TContext>& ser)
{
  ser.tag("array");
  Expects(ser.node.is_seq());
  Expects(ser.node.num_children() == N);
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    access<T>::callSerializeOrLoad(data[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
