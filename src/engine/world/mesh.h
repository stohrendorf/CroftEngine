#pragma once

#include "core/units.h"
#include "core/vec.h"

namespace loader::file
{
class RenderMeshData;
}

namespace engine::world
{
struct Mesh
{
  core::TRVec collisionCenter;
  core::Length collisionRadius;

  std::shared_ptr<loader::file::RenderMeshData> meshData{nullptr};
};
} // namespace engine::world
