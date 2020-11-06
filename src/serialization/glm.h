#pragma once

#include "serialization.h"

#include <glm/glm.hpp>

namespace serialization
{
inline void save(glm::mat4& m, const Serializer& ser)
{
  ser.tag("mat4");
  ser.node |= ryml::SEQ;
  for(int x = 0; x < 4; ++x)
    for(int y = 0; y < 4; ++y)
      ser.node.append_child() << m[x][y];
}

inline void load(glm::mat4& m, const Serializer& ser)
{
  ser.tag("mat4");
  Expects(ser.node.is_seq() && ser.node.num_children() == 4 * 4);
  auto it = ser.node.begin();
  for(int x = 0; x < 4; ++x)
  {
    for(int y = 0; y < 4; ++y)
    {
      *it >> m[x][y];
      ++it;
    }
  }
}

inline glm::mat4 create(const TypeId<glm::mat4>&, const Serializer& ser)
{
  ser.tag("mat4");
  Expects(ser.loading);
  glm::mat4 m{};
  load(m, ser);
  return m;
}
} // namespace serialization
