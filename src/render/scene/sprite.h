#pragma once

#include <array>
#include <gl/vertexbuffer.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>

namespace render::scene
{
class Material;
class Mesh;

struct SpriteVertex
{
  glm::vec3 pos;
  glm::vec3 uv;
  glm::vec4 color{1.0f};
  glm::vec3 normal{0, 0, 1};
  glm::vec4 reflective{0, 0, 0, 0};

  [[nodiscard]] static gl::VertexLayout<SpriteVertex> getLayout();
};

extern gslu::nn_shared<Mesh> createSpriteMesh(float x0,
                                              float y0,
                                              float x1,
                                              float y1,
                                              const glm::vec2& t0,
                                              const glm::vec2& t1,
                                              const gslu::nn_shared<Material>& materialFull,
                                              int textureIdx,
                                              const std::string& label);
} // namespace render::scene
