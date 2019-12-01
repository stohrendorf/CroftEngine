#pragma once

#include "serialization_fwd.h"

#include <array>

namespace serialization
{
template<typename T, size_t N>
void save(std::array<T, N>& data, const Serializer& ser)
{
  ser.tag("array");
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    const auto tmp = ser.withNode(YAML::Node{});
    access::callSerializeOrSave(data[i], tmp);
    ser.node.push_back(tmp.node);
  }
}

template<typename T, size_t N>
void load(std::array<T, N>& data, const Serializer& ser)
{
  ser.tag("array");
  Expects(ser.node.IsSequence());
  Expects(ser.node.size() == N);
  for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
  {
    access::callSerializeOrLoad(data[i], ser.withNode(ser.node[i]));
  }
}
} // namespace serialization
