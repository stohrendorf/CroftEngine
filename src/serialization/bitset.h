#pragma once

#include "serialization/serialization.h"

#include <bitset>

namespace serialization
{
template<size_t N>
void save(std::bitset<N>& data, const Serializer& ser)
{
  ser.tag("bitset");
  ser.node = YAML::Node{YAML::NodeType::Sequence};
  ser.node.SetStyle(YAML::EmitterStyle::Flow);
  for(size_t i = 0; i < N; ++i)
  {
    ser.node.push_back(data.test(i));
  }
}

template<size_t N>
void load(std::bitset<N>& data, const Serializer& ser)
{
  ser.tag("bitset");
  data.reset();
  Expects(ser.node.IsSequence());
  Expects(ser.node.size() == N);
  for(size_t i = 0; i < N; ++i)
  {
    if(ser.node[i].as<bool>())
      data.set(i);
  }
}
} // namespace serialization
