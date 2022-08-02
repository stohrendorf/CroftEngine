#include "sprite.h"

#include "material.h"
#include "materialgroup.h"
#include "mesh.h"
#include "names.h"
#include "rendermode.h"
#include "shaderprogram.h"

#include <boost/assert.hpp>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <gslu.h>
#include <vector>

namespace render::scene
{
namespace
{
std::array<SpriteVertex, 4> createSpriteVertices(const float x0,
                                                 const float y0,
                                                 const float x1,
                                                 const float y1,
                                                 const glm::vec2& t0,
                                                 const glm::vec2& t1,
                                                 const int textureIdx)
{
  BOOST_ASSERT(textureIdx >= 0);
  const std::array<SpriteVertex, 4> vertices{
    SpriteVertex{{x0, y0, 0}, {t0.x, t0.y, textureIdx}},
    SpriteVertex{{x1, y0, 0}, {t1.x, t0.y, textureIdx}},
    SpriteVertex{{x1, y1, 0}, {t1.x, t1.y, textureIdx}},
    SpriteVertex{{x0, y1, 0}, {t0.x, t1.y, textureIdx}},
  };
  return vertices;
}

gslu::nn_shared<gl::VertexBuffer<SpriteVertex>> createSpriteVertexBuffer(float x0,
                                                                         float y0,
                                                                         float x1,
                                                                         float y1,
                                                                         const glm::vec2& t0,
                                                                         const glm::vec2& t1,
                                                                         int textureIdx,
                                                                         bool instanced,
                                                                         const std::string& label)
{
  const auto vertices = createSpriteVertices(x0, y0, x1, y1, t0, t1, textureIdx);
  auto vb = gsl::make_shared<gl::VertexBuffer<SpriteVertex>>(
    instanced ? SpriteVertex::getInstancedLayout() : SpriteVertex::getLayout(), label + ":vb");
  vb->setData(vertices, gl::api::BufferUsage::StaticDraw);
  return vb;
}
} // namespace

gslu::nn_shared<Mesh> createSpriteMesh(const float x0,
                                       const float y0,
                                       const float x1,
                                       const float y1,
                                       const glm::vec2& t0,
                                       const glm::vec2& t1,
                                       const gslu::nn_shared<Material>& materialFull,
                                       const int textureIdx,
                                       const std::string& label)
{
  auto vb = createSpriteVertexBuffer(x0, y0, x1, y1, t0, t1, textureIdx, false, label);
  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>(label + ":idx");
  indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);

  auto vao = gsl::make_shared<gl::VertexArray<uint16_t, SpriteVertex>>(
    indexBuffer, vb, std::vector{&materialFull->getShaderProgram()->getHandle()}, label + ":va");
  auto mesh = gsl::make_shared<MeshImpl<uint16_t, SpriteVertex>>(vao);
  mesh->getMaterialGroup().set(RenderMode::Full, materialFull);
  mesh->getRenderState().setScissorTest(false);

  return mesh;
}

std::tuple<gslu::nn_shared<Mesh>, gslu::nn_shared<gl::VertexBuffer<glm::mat4>>>
  createInstancedSpriteMesh(const float x0,
                            const float y0,
                            const float x1,
                            const float y1,
                            const glm::vec2& t0,
                            const glm::vec2& t1,
                            const gslu::nn_shared<Material>& materialFull,
                            const int textureIdx,
                            const std::string& label)
{
  const auto vertices = createSpriteVertexBuffer(x0, y0, x1, y1, t0, t1, textureIdx, true, label);

  static const gl::VertexLayout<glm::mat4> layout{
    {VERTEX_ATTRIBUTE_MODEL_MATRIX_NAME, gl::VertexAttribute<glm::mat4>::Single{}}};
  const auto modelMatrices = gsl::make_shared<gl::VertexBuffer<glm::mat4>>(layout, label + ":matrices", 1);
  modelMatrices->setData(std::vector<glm::mat4>(4096, glm::mat4{0.0f}), gl::api::BufferUsage::StreamDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};
  auto indexBuffer = gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>(label + ":idx");
  indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);

  auto vao = gsl::make_shared<gl::VertexArray<uint16_t, SpriteVertex, glm::mat4>>(
    indexBuffer,
    std::tuple{vertices, modelMatrices},
    std::vector{&materialFull->getShaderProgram()->getHandle()},
    label + ":va");
  auto mesh = gsl::make_shared<MeshImpl<uint16_t, SpriteVertex, glm::mat4>>(vao);
  mesh->getMaterialGroup().set(RenderMode::Full, materialFull);
  mesh->getRenderState().setScissorTest(false);

  return {mesh, modelMatrices};
}

gl::VertexLayout<SpriteVertex> SpriteVertex::getLayout()
{
  return {
    {VERTEX_ATTRIBUTE_POSITION_NAME, &SpriteVertex::pos},
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &SpriteVertex::uv},
    {VERTEX_ATTRIBUTE_COLOR_NAME, &SpriteVertex::color},
    {VERTEX_ATTRIBUTE_NORMAL_NAME, &SpriteVertex::normal},
    {VERTEX_ATTRIBUTE_REFLECTIVE_NAME, &SpriteVertex::reflective},
  };
}

gl::VertexLayout<SpriteVertex> SpriteVertex::getInstancedLayout()
{
  return {
    {VERTEX_ATTRIBUTE_POSITION_NAME, &SpriteVertex::pos},
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &SpriteVertex::uv},
    {VERTEX_ATTRIBUTE_COLOR_NAME, &SpriteVertex::color},
    {VERTEX_ATTRIBUTE_NORMAL_NAME, &SpriteVertex::normal},
  };
}
} // namespace render::scene
