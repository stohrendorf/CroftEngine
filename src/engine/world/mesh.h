#pragma once

#include "core/units.h"
#include "core/vec.h"

namespace engine::world
{
class RenderMeshData;

struct Mesh
{
  core::TRVec collisionCenter;
  core::Length collisionRadius;

  std::shared_ptr<RenderMeshData> meshData{nullptr};
};
} // namespace engine::world
