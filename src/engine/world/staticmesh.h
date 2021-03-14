#pragma once

#include "core/boundingbox.h"
#include "core/id.h"

namespace render::scene
{
class Mesh;
}

namespace engine::world
{
struct StaticMesh
{
  const core::BoundingBox collisionBox;
  const bool doNotCollide;
  const bool isVisible;

  std::shared_ptr<render::scene::Mesh> renderMesh{nullptr};
};
} // namespace engine::world
