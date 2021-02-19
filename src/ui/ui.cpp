#include "ui.h"

#include "boxgouraud.h"
#include "render/scene/material.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/rendercontext.h"

#include <gl/debuggroup.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <glm/glm.hpp>

namespace ui
{
namespace
{
struct QuadVertex
{
  glm::vec2 pos;
  glm::vec4 color;
  glm::vec2 uv{0};
  const glm::int32_t texIndex{-1};
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const std::shared_ptr<render::scene::Material>& material,
                                                               const std::array<QuadVertex, 4>& vertices)
{
  static const gl::VertexFormat<QuadVertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &QuadVertex::pos},
                                                   {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &QuadVertex::uv},
                                                   {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &QuadVertex::texIndex},
                                                   {VERTEX_ATTRIBUTE_COLOR_NAME, &QuadVertex::color}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<QuadVertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, QuadVertex>>(
    std::make_shared<gl::VertexArray<uint16_t, QuadVertex>>(
      indexBuffer, vertexBuffer, std::vector{&material->getShaderProgram()->getHandle()}));
  mesh->getMaterial().set(render::scene::RenderMode::Full, material);
  return mesh;
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const glm::vec2& topLeft,
                                                               const glm::vec2& bottomRight,
                                                               const BoxGouraud& colors,
                                                               const std::shared_ptr<render::scene::Material>& material)
{
  static const auto toColor = [](const gl::SRGBA8& c) { return glm::vec4{c.channels} / 255.0f; };

  return createQuad(material,
                    {QuadVertex{{topLeft.x, topLeft.y}, toColor(colors.topLeft)},
                     QuadVertex{{topLeft.x, bottomRight.y}, toColor(colors.bottomLeft)},
                     QuadVertex{{bottomRight.x, bottomRight.y}, toColor(colors.bottomRight)},
                     QuadVertex{{bottomRight.x, topLeft.y}, toColor(colors.topRight)}});
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const glm::vec2& a,
                                                               const glm::vec2& dxy,
                                                               const gl::SRGBA8& color,
                                                               const std::shared_ptr<render::scene::Material>& material)
{
  static const auto toColor = [](const gl::SRGBA8& c) { return glm::vec4{c.channels} / 255.0f; };

  const auto b = a + dxy;

  return createQuad(material,
                    {QuadVertex{{a.x, a.y}, toColor(color)},
                     QuadVertex{{a.x, b.y}, toColor(color)},
                     QuadVertex{{b.x, b.y}, toColor(color)},
                     QuadVertex{{b.x, a.y}, toColor(color)}});
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createHLine(
  const glm::vec2& a, int length, const gl::SRGBA8& color, const std::shared_ptr<render::scene::Material>& material)
{
  /*
  if(length < 0)
    --length;
  else
    ++length;
*/
  return createQuad(a, glm::vec2{length, 1}, color, material);
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createVLine(
  const glm::vec2& a, int length, const gl::SRGBA8& color, const std::shared_ptr<render::scene::Material>& material)
{
  /*
  if(length < 0)
    --length;
  else
    ++length;
*/
  return createQuad(a, glm::vec2{1, length}, color, material);
};
} // namespace

void Ui::drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size)
{
  const auto color1 = m_palette[15];
  const auto color2 = m_palette[31];

  // top
  m_meshes.emplace_back(createHLine(xy - glm::ivec2{0, 1}, size.x + 1, color1, m_material));
  m_meshes.emplace_back(createHLine(xy, size.x, color2, m_material));
  //right
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{size.x, -1}, size.y + 2, color1, m_material));
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{size.x + 1, -1}, size.y + 3, color2, m_material));
  // bottom
  m_meshes.emplace_back(createHLine(xy + size, -size.x, color1, m_material));
  m_meshes.emplace_back(createHLine(xy + size + glm::ivec2{1, 1}, -size.x - 2, color2, m_material));
  // left
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{-1, size.y + 1}, -size.y - 2, color1, m_material));
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{0, size.y}, -size.y, color2, m_material));
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud)
{
  Expects(size.x >= 0 && size.y >= 0);

  m_meshes.emplace_back(createQuad(xy, xy + size, gouraud, m_material));
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color)
{
  Expects(size.x >= 0 && size.y >= 0);

  m_meshes.emplace_back(createQuad(xy, size, color, m_material));
}

void Ui::render()
{
  SOGLB_DEBUGGROUP("ui");
  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};
  for(const auto& mesh : m_meshes)
    mesh->render(ctx);
  m_meshes.clear();
}
} // namespace ui
