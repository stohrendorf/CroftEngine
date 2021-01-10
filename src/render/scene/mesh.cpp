#include "mesh.h"

#include "material.h"
#include "names.h"

namespace render::scene
{
gsl::not_null<std::shared_ptr<Mesh>>
  createQuadFullscreen(const float width, const float height, const gl::Program& program, const bool invertY)
{
  struct Vertex
  {
    glm::vec2 pos;

    glm::vec2 uv;
  };

  const std::array<Vertex, 4> vertices{Vertex{{0.0f, 0.0f}, {0.0f, invertY ? 1.0f : 0.0f}},
                                       Vertex{{width, 0.0f}, {1.0f, invertY ? 1.0f : 0.0f}},
                                       Vertex{{width, height}, {1.0f, invertY ? 0.0f : 1.0f}},
                                       Vertex{{0.0f, height}, {0.0f, invertY ? 0.0f : 1.0f}}};

  static const gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                               {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  return std::make_shared<MeshImpl<uint16_t, Vertex>>(
    std::make_shared<gl::VertexArray<uint16_t, Vertex>>(indexBuffer, vertexBuffer, std::vector{&program}));
}

Mesh::~Mesh() = default;

bool Mesh::render(RenderContext& context)
{
  std::shared_ptr<Material> material = m_material.get(context.getRenderMode());
  if(material == nullptr)
    return false;

  BOOST_ASSERT(context.getCurrentNode() != nullptr);

  context.pushState(getRenderState());
  context.pushState(material->getRenderState());
  context.bindState();

  material->bind(*context.getCurrentNode());

  drawIndexBuffer(m_primitiveType);

  context.popState();
  context.popState();
  return true;
}
} // namespace render::scene
