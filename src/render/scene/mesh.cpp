#include "mesh.h"

#include "names.h"
#include "render/material/material.h"
#include "render/material/materialgroup.h"
#include "render/material/rendermode.h"
#include "render/material/shaderprogram.h"
#include "rendercontext.h"
#include "translucency.h"

#include <array>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/constants.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <gslu.h>
#include <vector>

namespace render::scene
{
class Node;

gslu::nn_shared<Mesh> createScreenQuad(const glm::vec2& xy,
                                       const glm::vec2& size,
                                       const std::shared_ptr<material::Material>& material,
                                       Translucency spriteTranslucency,
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

  auto vertexBuffer = gsl::make_shared<gl::VertexBuffer<Vertex>>(
    layout, label + gl::VboSuffix, gl::api::BufferUsage::StaticDraw, vertices);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>(
    label + gl::IndexBufferSuffix, gl::api::BufferUsage::StaticDraw, indices);

  const auto vao = std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vertexBuffer, std::vector{&material->getShaderProgram()->getHandle()}, label + gl::VaoSuffix);

  std::remove_const_t<decltype(vao)> opaqueVao;
  std::remove_const_t<decltype(vao)> nonOpaqueVao;
  switch(spriteTranslucency)
  {
  case Translucency::Opaque:
    opaqueVao = vao;
    break;
  case Translucency::NonOpaque:
    nonOpaqueVao = vao;
    break;
  }

  auto mesh = gsl::make_shared<MeshImpl<uint16_t, Vertex>>(opaqueVao, nonOpaqueVao, gl::api::PrimitiveType::Triangles);
  mesh->getRenderState().setCullFace(false);
  mesh->getRenderState().setDepthWrite(false);
  mesh->getRenderState().setDepthTest(false);
  mesh->getMaterialGroup().set(spriteTranslucency == Translucency::Opaque ? material::RenderMode::FullOpaque
                                                                          : material::RenderMode::FullNonOpaque,
                               material);
  return mesh;
}

Mesh::~Mesh() = default;

void Mesh::render(const Node* node, RenderContext& context)
{
  const std::shared_ptr<material::Material> material = m_materialGroup.get(context.getRenderMode());
  if(material == nullptr)
    return;

  context.pushState(material->getRenderState());
  context.pushState(getRenderState());
  context.bindState();

  material->bind(node, *this);

  drawElements(context.getTranslucencySelector());

  context.popState();
  context.popState();
}

void Mesh::render(const Node* node, RenderContext& context, gl::api::core::SizeType instanceCount)
{
  const std::shared_ptr<material::Material> material = m_materialGroup.get(context.getRenderMode());
  if(material == nullptr)
    return;

  context.pushState(material->getRenderState());
  context.pushState(getRenderState());
  context.bindState();

  material->bind(node, *this);

  drawElements(context.getTranslucencySelector(), instanceCount);

  context.popState();
  context.popState();
}
} // namespace render::scene
