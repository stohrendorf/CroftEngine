#pragma once

#include "core/units.h"
#include "core/vec.h"

#include <gsl/gsl-lite.hpp>

namespace engine::world
{
class RenderMeshData;

struct Mesh
{
  core::TRVec collisionCenter;
  core::Length collisionRadius;
  gsl::not_null<std::shared_ptr<RenderMeshData>> meshData;
};
} // namespace engine::world
