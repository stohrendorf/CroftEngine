#pragma once

#include "core/id.h"

#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace engine::world
{
struct Sprite
{
  core::TextureId textureId{uint16_t(0)};

  glm::vec2 uv0;
  glm::vec2 uv1;

  glm::ivec2 render0;
  glm::ivec2 render1;
};

struct SpriteSequence
{
  core::TypeId type{uint16_t(0)};
  gsl::span<const Sprite> sprites{};
};
} // namespace engine::world
