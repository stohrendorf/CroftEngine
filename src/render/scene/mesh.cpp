#include "mesh.h"

#include "material.h"
#include "materialgroup.h"
#include "names.h"
#include "rendercontext.h"
#include "rendermode.h"
#include "shaderprogram.h"

#include <array>
#include <boost/assert.hpp>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <vector>

namespace render::scene
{
gsl::not_null<std::shared_ptr<Mesh>> createScreenQuad(const glm::vec2& xy,
                                                      const glm::vec2& size,
                                                      const std::shared_ptr<Material>& material,
                                                      const std::string& label)
{
  struct Vertex
  {
    glm::vec2 pos;
    glm::vec2 uv;
  };

  const auto tl = xy;
  const auto br = xy + size;
  const std::array<Vertex, 4> vertices{Vertex{{tl.x, tl.y}, {0.0f, 0.0f}},
                                       Vertex{{br.x, tl.y}, {1.0f, 0.0f}},
                                       Vertex{{br.x, br.y}, {1.0f, 1.0f}},
                                       Vertex{{tl.x, br.y}, {0.0f, 1.0f}}};

  static const gl::VertexLayout<Vertex> layout{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                               {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<Vertex>>(layout, 0, label + "-vbo");
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsage::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>(label + "-idx");
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsage::StaticDraw);

  auto mesh = std::make_shared<MeshImpl<uint16_t, Vertex>>(std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vertexBuffer, std::vector{&material->getShaderProgram()->getHandle()}, label + "-vtx"));
  mesh->getRenderState().setCullFace(false);
  mesh->getRenderState().setDepthWrite(false);
  mesh->getRenderState().setDepthTest(false);
  mesh->getMaterialGroup().set(RenderMode::Full, material);
  return mesh;
}

Mesh::~Mesh() = default;

bool Mesh::render(RenderContext& context)
{
  std::shared_ptr<Material> material = m_materialGroup.get(context.getRenderMode());
  if(material == nullptr)
    return false;

  BOOST_ASSERT(context.getCurrentNode() != nullptr);

  context.pushState(getRenderState());
  context.pushState(material->getRenderState());
  context.bindState();

  material->bind(*context.getCurrentNode(), *this);

  drawIndexBuffer(m_primitiveType);

  context.popState();
  context.popState();
  return true;
}
} // namespace render::scene
