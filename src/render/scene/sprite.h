#pragma once

#include <gl/vertexbuffer.h>
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>

namespace render::scene
{
class Material;
class Mesh;
class Node;

struct SpriteVertex
{
  glm::vec3 pos;
  glm::vec2 uv;
  int textureIdx;
  glm::vec4 color{1.0f};
  glm::vec3 normal{0, 0, 1};

  [[nodiscard]] static gl::VertexLayout<SpriteVertex> getLayout();
};

extern std::array<SpriteVertex, 4> createSpriteVertices(
  float x0, float y0, float x1, float y1, const glm::vec2& t0, const glm::vec2& t1, int textureIdx);

extern std::shared_ptr<gl::VertexBuffer<SpriteVertex>> createSpriteVertexBuffer(
  float x0, float y0, float x1, float y1, const glm::vec2& t0, const glm::vec2& t1, int textureIdx);

extern gsl::not_null<std::shared_ptr<Mesh>>
  createSpriteMesh(float x0,
                   float y0,
                   float x1,
                   float y1,
                   const glm::vec2& t0,
                   const glm::vec2& t1,
                   const gsl::not_null<std::shared_ptr<Material>>& materialFull,
                   int textureIdx);
} // namespace render::scene
