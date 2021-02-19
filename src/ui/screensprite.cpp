#include "screensprite.h"

#include "render/scene/material.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/rendercontext.h"

#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>

namespace ui
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>> ScreenSprite::createQuad(const glm::vec2& xy,
                                                                             const gl::Program& program)
{
  struct Vertex
  {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::int32_t texIndex;
    glm::vec4 color{0.0f};
  };

  const auto pa = glm::vec2{m_sprite.render0} + xy;
  const auto pb = glm::vec2{m_sprite.render1} + xy;
  const auto ua = m_sprite.uv0.toGl();
  const auto ub = m_sprite.uv1.toGl();

  const std::array<Vertex, 4> vertices{Vertex{{pa.x, pa.y}, {ua.x, ua.y}, m_sprite.texture_id.get()},
                                       Vertex{{pa.x, pb.y}, {ua.x, ub.y}, m_sprite.texture_id.get()},
                                       Vertex{{pb.x, pb.y}, {ub.x, ub.y}, m_sprite.texture_id.get()},
                                       Vertex{{pb.x, pa.y}, {ub.x, ua.y}, m_sprite.texture_id.get()}};

  static const gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                               {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv},
                                               {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &Vertex::texIndex},
                                               {VERTEX_ATTRIBUTE_COLOR_NAME, &Vertex::color}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  return std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(
    std::make_shared<gl::VertexArray<uint16_t, Vertex>>(indexBuffer, vertexBuffer, std::vector{&program}));
}

void ScreenSprite::render(const glm::vec2& xy,
                          const glm::vec2& screenSize,
                          const std::shared_ptr<render::scene::Material>& material)
{
  auto quad = createQuad(xy, material->getShaderProgram()->getHandle());
  quad->getMaterial().set(render::scene::RenderMode::Full, material);
  quad->getMaterial().get(render::scene::RenderMode::Full)->getUniform("u_screenSize")->set(screenSize);
  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
  quad->render(context);
}
} // namespace ui
