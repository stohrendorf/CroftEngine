#pragma once

#include "core/vec.h"

namespace loader::file
{
inline glm::vec3 generateNormal(const glm::vec3& o, const glm::vec3& a, const glm::vec3& b)
{
  return normalize(-cross(a - o, b - o));
}

inline glm::vec3 generateNormal(const core::TRVec& o, const core::TRVec& a, const core::TRVec& b)
{
  return generateNormal(o.toRenderSystem(), a.toRenderSystem(), b.toRenderSystem());
}
} // namespace loader::file
