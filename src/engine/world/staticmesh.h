#pragma once

#include "core/boundingbox.h"

#include <memory>

namespace render::scene
{
class Mesh;
}

namespace engine::world
{
struct StaticMesh
{
  core::BoundingBox collisionBox;
  bool doNotCollide = false;

  std::shared_ptr<render::scene::Mesh> renderMesh{nullptr};
};
} // namespace engine::world
