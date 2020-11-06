#pragma once

#include "serialization_fwd.h"

#include <array>

namespace serialization
{
template<typename T, size_t N>
void save(std::array<T, N>& data, const Serializer& ser)
{
  ser.tag("array");
  ser.node |= ryml::SEQ;
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    const auto tmp = ser.newChild();
    access::callSerializeOrSave(data[i], tmp);
  }
}

template<typename T, size_t N>
void load(std::array<T, N>& data, const Serializer& ser)
{
  ser.tag("array");
  Expects(ser.node.is_seq());
  Expects(ser.node.num_children() == N);
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    access::callSerializeOrLoad(data[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
