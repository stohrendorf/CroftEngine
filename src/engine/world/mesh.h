#pragma once

#include "core/units.h"
#include "core/vec.h"

#include <gsl/gsl-lite.hpp>
#include <gslu.h>

namespace engine::world
{
class RenderMeshData;

struct Mesh
{
  core::TRVec collisionCenter;
  core::Length collisionRadius;
  gslu::nn_shared<RenderMeshData> meshData;
};
} // namespace engine::world
