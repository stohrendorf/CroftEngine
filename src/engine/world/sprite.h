#pragma once

#include "core/id.h"

#include <cstdint>
#include <gl/soglb_fwd.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <tuple>

namespace render::scene
{
class Mesh;
}

namespace engine::world
{
struct Sprite
{
  core::AtlasId atlasId{uint16_t(0)};

  glm::vec2 uv0;
  glm::vec2 uv1;

  glm::ivec2 render0;
  glm::ivec2 render1;

  std::shared_ptr<render::scene::Mesh> yBoundMesh;
  std::shared_ptr<render::scene::Mesh> billboardMesh;
  std::tuple<std::shared_ptr<render::scene::Mesh>, std::shared_ptr<gl::VertexBuffer<glm::mat4>>> instancedBillboardMesh;

  constexpr bool operator==(const Sprite& rhs) const
  {
    return atlasId == rhs.atlasId && uv0 == rhs.uv0 && uv1 == rhs.uv1 && render0 == rhs.render0
           && render1 == rhs.render1;
  }
};

struct SpriteSequence
{
  core::TypeId type{uint16_t(0)};
  gsl::span<const Sprite> sprites;
};
} // namespace engine::world
