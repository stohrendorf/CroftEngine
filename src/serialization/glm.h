#pragma once

#include "serialization_fwd.h"

#include <glm/glm.hpp>
#include <ryml.hpp>

namespace serialization
{
template<typename TContext>
inline void serialize(const glm::mat4& m, const Serializer<TContext>& ser)
{
  ser.tag("mat4");
  ser.node |= ryml::SEQ;
  for(int x = 0; x < 4; ++x)
    for(int y = 0; y < 4; ++y)
      ser.node.append_child() << m[x][y];
}

template<typename TContext>
inline void deserialize(glm::mat4& m, const Deserializer<TContext>& ser)
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

template<typename TContext>
inline glm::mat4 create(const TypeId<glm::mat4>&, const Deserializer<TContext>& ser)
{
  ser.tag("mat4");
  glm::mat4 m{};
  deserialize(m, ser);
  return m;
}

template<typename TContext, glm::length_t L, typename T, glm::qualifier Q>
inline void serialize(const glm::vec<L, T, Q>& v, const Serializer<TContext>& ser)
{
  ser.tag("vec");
  ser.node |= ryml::SEQ;
  for(glm::length_t x = 0; x < L; ++x)
    ser.node.append_child() << v[x];
}

template<typename TContext, glm::length_t L, typename T, glm::qualifier Q>
inline void deserialize(glm::vec<L, T, Q>& v, const Deserializer<TContext>& ser)
{
  ser.tag("vec");
  Expects(ser.node.is_seq() && ser.node.num_children() == L);
  auto it = ser.node.begin();
  for(glm::length_t x = 0; x < L; ++x)
  {
    *it >> v[x];
    ++it;
  }
}

template<typename TContext, glm::length_t L, typename T, glm::qualifier Q>
inline glm::vec<L, T, Q> create(const TypeId<glm::vec<L, T, Q>>&, const Deserializer<TContext>& ser)
{
  ser.tag("vec");
  glm::vec<L, T, Q> v{};
  deserialize(v, ser);
  return v;
}
} // namespace serialization
