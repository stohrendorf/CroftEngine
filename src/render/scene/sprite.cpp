#include "sprite.h"

#include "camera.h"
#include "material.h"
#include "mesh.h"
#include "names.h"
#include "node.h"

#include <gl/vertexarray.h>

namespace render::scene
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
  const std::array<SpriteVertex, 4> vertices{SpriteVertex{{x0, y0, 0}, {t0.x, t0.y}, textureIdx},
                                             SpriteVertex{{x1, y0, 0}, {t1.x, t0.y}, textureIdx},
                                             SpriteVertex{{x1, y1, 0}, {t1.x, t1.y}, textureIdx},
                                             SpriteVertex{{x0, y1, 0}, {t0.x, t1.y}, textureIdx}};
  return vertices;
}

std::shared_ptr<gl::VertexBuffer<SpriteVertex>> createSpriteVertexBuffer(float x0,
                                                                         float y0,
                                                                         float x1,
                                                                         float y1,
                                                                         const glm::vec2& t0,
                                                                         const glm::vec2& t1,
                                                                         int textureIdx,
                                                                         const std::string& label)
{
  const auto vertices = createSpriteVertices(x0, y0, x1, y1, t0, t1, textureIdx);
  auto vb = std::make_shared<gl::VertexBuffer<SpriteVertex>>(SpriteVertex::getLayout(), 0, label);
  vb->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);
  return vb;
}

gsl::not_null<std::shared_ptr<Mesh>> createSpriteMesh(const float x0,
                                                      const float y0,
                                                      const float x1,
                                                      const float y1,
                                                      const glm::vec2& t0,
                                                      const glm::vec2& t1,
                                                      const gsl::not_null<std::shared_ptr<Material>>& materialFull,
                                                      const int textureIdx,
                                                      const std::string& label)
{
  auto vb = createSpriteVertexBuffer(x0, y0, x1, y1, t0, t1, textureIdx, label);
  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(gsl::not_null(&indices[0]), 6, gl::api::BufferUsageARB::StaticDraw);

  auto vao = std::make_shared<gl::VertexArray<uint16_t, SpriteVertex>>(
    indexBuffer, vb, std::vector{&materialFull->getShaderProgram()->getHandle()}, label);
  auto mesh = std::make_shared<MeshImpl<uint16_t, SpriteVertex>>(vao);
  mesh->getMaterialGroup().set(RenderMode::Full, materialFull);

  return mesh;
}

gl::VertexLayout<SpriteVertex> SpriteVertex::getLayout()
{
  return {{VERTEX_ATTRIBUTE_POSITION_NAME, &SpriteVertex::pos},
          {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &SpriteVertex::uv},
          {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &SpriteVertex::textureIdx},
          {VERTEX_ATTRIBUTE_COLOR_NAME, &SpriteVertex::color},
          {VERTEX_ATTRIBUTE_NORMAL_NAME, &SpriteVertex::normal}};
}
} // namespace render::scene
