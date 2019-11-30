#pragma once

#include "serialization.h"

#include <glm/glm.hpp>

namespace serialization
{
inline void save(glm::mat4& m, const Serializer& ser)
{
  ser.node = YAML::Node{YAML::NodeType::Sequence};
  ser.node.SetStyle(YAML::EmitterStyle::Flow);
  for(int x = 0; x < 4; ++x)
    for(int y = 0; y < 4; ++y)
      ser.node.push_back(m[x][y]);
}

inline void load(glm::mat4& m, const Serializer& ser)
{
  Expects(ser.node.IsSequence() && ser.node.size() == 4 * 4);
  auto it = ser.node.begin();
  for(int x = 0; x < 4; ++x)
    for(int y = 0; y < 4; ++y)
      m[x][y] = (it++)->as<float>();
}

inline glm::mat4 create(const TypeId<glm::mat4>&, const Serializer& ser)
{
  Expects(ser.loading);
  glm::mat4 m{};
  load(m, ser);
  return m;
}
} // namespace serialization
