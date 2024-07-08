#pragma once

#include "core/vec.h"

#include <glm/vec3.hpp>

namespace engine::world
{
inline glm::vec3 generateNormal(const glm::vec3& o, const glm::vec3& a, const glm::vec3& b)
{
  return normalize(-cross(a - o, b - o));
}

inline glm::vec3 generateNormal(const core::TRVec& o, const core::TRVec& a, const core::TRVec& b)
{
  return generateNormal(o.toRenderSystem(), a.toRenderSystem(), b.toRenderSystem());
}

inline bool isDistortedQuad(const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3, const glm::vec2& v4)
{
  const auto e1 = v2 - v1;
  const auto e2 = v3 - v2;
  const auto e3 = v4 - v3;
  const auto e4 = v1 - v4;
  static constexpr auto Eps = float(1e-14);
  return glm::abs(glm::dot(e1, e2)) > Eps || glm::abs(glm::dot(e2, e3)) > Eps || glm::abs(glm::dot(e3, e4)) > Eps
         || glm::abs(glm::dot(e4, e1)) > Eps;
}
} // namespace engine::world
