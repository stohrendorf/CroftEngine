#pragma once

#include "mesh.h"
#include "renderable.h"

#include <gsl-lite.hpp>

namespace render::scene
{
class Material;

enum class SpritePole
{
  X,
  Y,
  Z
};

extern gsl::not_null<std::shared_ptr<Mesh>>
  createSpriteMesh(float x0,
                   float y0,
                   float x1,
                   float y1,
                   const glm::vec2& t0,
                   const glm::vec2& t1,
                   const gsl::not_null<std::shared_ptr<Material>>& materialFull);

extern void bindSpritePole(Node& node, SpritePole pole);
} // namespace render::scene
