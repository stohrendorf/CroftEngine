#pragma once

#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace render::scene
{
class Material;
class Mesh;
class Node;

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
                   const gsl::not_null<std::shared_ptr<Material>>& materialFull,
                   int textureIdx);

extern void bindSpritePole(Node& node, SpritePole pole);
} // namespace render::scene
