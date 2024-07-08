#pragma once

#include <cstdint>
#include <gl/vertexbuffer.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gslu.h>
#include <string>
#include <tuple>

namespace render::material
{
class Material;
enum class RenderMode : uint8_t;
} // namespace render::material

namespace render::scene
{
class Mesh;

struct SpriteVertex
{
  glm::vec3 pos;
  glm::vec3 uv;
  glm::vec4 color{1.0f};
  glm::vec3 normal{0, 0, 1};
  glm::vec4 reflective{0, 0, 0, 0};

  [[nodiscard]] static gl::VertexLayout<SpriteVertex> getLayout();
  [[nodiscard]] static gl::VertexLayout<SpriteVertex> getInstancedLayout();
};

extern gslu::nn_shared<Mesh> createSpriteMesh(float x0,
                                              float y0,
                                              float x1,
                                              float y1,
                                              const glm::vec2& t0,
                                              const glm::vec2& t1,
                                              material::RenderMode renderMode,
                                              const gslu::nn_shared<material::Material>& materialFull,
                                              int textureIdx,
                                              const std::string& label);

extern std::tuple<gslu::nn_shared<Mesh>, gslu::nn_shared<gl::VertexBuffer<glm::mat4>>>
  createInstancedSpriteMesh(float x0,
                            float y0,
                            float x1,
                            float y1,
                            const glm::vec2& t0,
                            const glm::vec2& t1,
                            material::RenderMode renderMode,
                            const gslu::nn_shared<material::Material>& materialFull,
                            int textureIdx,
                            const std::string& label);
} // namespace render::scene
