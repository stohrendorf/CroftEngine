#include "ui.h"

#include "boxgouraud.h"
#include "loader/file/datatypes.h"
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
struct ColorQuadVertex
{
  glm::vec2 pos;
  glm::vec2 uv;
  glm::vec4 topLeft;
  glm::vec4 topRight;
  glm::vec4 bottomLeft;
  glm::vec4 bottomRight;
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const std::shared_ptr<render::scene::Material>& material,
                                                               const std::array<ColorQuadVertex, 4>& vertices)
{
  static const gl::VertexFormat<ColorQuadVertex> format{
    {VERTEX_ATTRIBUTE_POSITION_NAME, &ColorQuadVertex::pos},
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &ColorQuadVertex::uv},
    {VERTEX_ATTRIBUTE_COLOR_TOP_LEFT_NAME, &ColorQuadVertex::topLeft},
    {VERTEX_ATTRIBUTE_COLOR_TOP_RIGHT_NAME, &ColorQuadVertex::topRight},
    {VERTEX_ATTRIBUTE_COLOR_BOTTOM_LEFT_NAME, &ColorQuadVertex::bottomLeft},
    {VERTEX_ATTRIBUTE_COLOR_BOTTOM_RIGHT_NAME, &ColorQuadVertex::bottomRight}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<ColorQuadVertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, ColorQuadVertex>>(
    std::make_shared<gl::VertexArray<uint16_t, ColorQuadVertex>>(
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
                    {ColorQuadVertex{{topLeft.x, topLeft.y},
                                     {0, 0},
                                     toColor(colors.topLeft),
                                     toColor(colors.topRight),
                                     toColor(colors.bottomLeft),
                                     toColor(colors.bottomRight)},
                     ColorQuadVertex{{topLeft.x, bottomRight.y},
                                     {0, 1},
                                     toColor(colors.topLeft),
                                     toColor(colors.topRight),
                                     toColor(colors.bottomLeft),
                                     toColor(colors.bottomRight)},
                     ColorQuadVertex{{bottomRight.x, bottomRight.y},
                                     {1, 1},
                                     toColor(colors.topLeft),
                                     toColor(colors.topRight),
                                     toColor(colors.bottomLeft),
                                     toColor(colors.bottomRight)},
                     ColorQuadVertex{{bottomRight.x, topLeft.y},
                                     {1, 0},
                                     toColor(colors.topLeft),
                                     toColor(colors.topRight),
                                     toColor(colors.bottomLeft),
                                     toColor(colors.bottomRight)}});
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const glm::vec2& a,
                                                               const glm::vec2& dxy,
                                                               const gl::SRGBA8& color,
                                                               const std::shared_ptr<render::scene::Material>& material)
{
  const auto glColor = glm::vec4{color.channels} / 255.0f;

  const auto b = a + dxy;

  return createQuad(material,
                    {ColorQuadVertex{{a.x, a.y}, {0, 0}, glColor, glColor, glColor, glColor},
                     ColorQuadVertex{{a.x, b.y}, {0, 1}, glColor, glColor, glColor, glColor},
                     ColorQuadVertex{{b.x, b.y}, {1, 1}, glColor, glColor, glColor, glColor},
                     ColorQuadVertex{{b.x, a.y}, {1, 0}, glColor, glColor, glColor, glColor}});
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createHLine(
  const glm::vec2& a, int length, const gl::SRGBA8& color, const std::shared_ptr<render::scene::Material>& material)
{
  return createQuad(a, glm::vec2{length, 1}, color, material);
};

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createVLine(
  const glm::vec2& a, int length, const gl::SRGBA8& color, const std::shared_ptr<render::scene::Material>& material)
{
  return createQuad(a, glm::vec2{1, length}, color, material);
};
} // namespace

void Ui::drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size)
{
  const auto color1 = m_palette[15];
  const auto color2 = m_palette[31];

  // top
  m_meshes.emplace_back(createHLine(xy - glm::ivec2{0, 1}, size.x + 1, color1, m_color));
  m_meshes.emplace_back(createHLine(xy, size.x, color2, m_color));
  //right
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{size.x, -1}, size.y + 2, color1, m_color));
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{size.x + 1, -1}, size.y + 3, color2, m_color));
  // bottom
  m_meshes.emplace_back(createHLine(xy + size, -size.x, color1, m_color));
  m_meshes.emplace_back(createHLine(xy + size + glm::ivec2{1, 1}, -size.x - 2, color2, m_color));
  // left
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{-1, size.y + 1}, -size.y - 2, color1, m_color));
  m_meshes.emplace_back(createVLine(xy + glm::ivec2{0, size.y}, -size.y, color2, m_color));
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud)
{
  Expects(size.x >= 0 && size.y >= 0);

  m_meshes.emplace_back(createQuad(xy, xy + size, gouraud, m_color));
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color)
{
  Expects(size.x >= 0 && size.y >= 0);

  m_meshes.emplace_back(createQuad(xy, size, color, m_color));
}

void Ui::render(const glm::vec2& screenSize)
{
  SOGLB_DEBUGGROUP("ui");
  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};
  for(const auto& mesh : m_meshes)
  {
    mesh->getMaterial().get(render::scene::RenderMode::Full)->getUniform("u_screenSize")->set(screenSize);
    mesh->render(ctx);
  }
  m_meshes.clear();
}

void Ui::draw(const loader::file::Sprite& sprite, const glm::ivec2& xy)
{
  struct TextureQuadVertex
  {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::int32_t texIndex;
  };

  static const gl::VertexFormat<TextureQuadVertex> format{
    {VERTEX_ATTRIBUTE_POSITION_NAME, &TextureQuadVertex::pos},
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &TextureQuadVertex::uv},
    {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &TextureQuadVertex::texIndex}};

  const auto a = sprite.render0 + xy;
  const auto b = sprite.render1 + xy;
  const auto ta = sprite.uv0.toGl();
  const auto tb = sprite.uv1.toGl();
  const std::array<TextureQuadVertex, 4> vertices{TextureQuadVertex{{a.x, a.y}, {ta.x, ta.y}, sprite.texture_id.get()},
                                                  TextureQuadVertex{{a.x, b.y}, {ta.x, tb.y}, sprite.texture_id.get()},
                                                  TextureQuadVertex{{b.x, b.y}, {tb.x, tb.y}, sprite.texture_id.get()},
                                                  TextureQuadVertex{{b.x, a.y}, {tb.x, ta.y}, sprite.texture_id.get()}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<TextureQuadVertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, TextureQuadVertex>>(
    std::make_shared<gl::VertexArray<uint16_t, TextureQuadVertex>>(
      indexBuffer, vertexBuffer, std::vector{&m_texture->getShaderProgram()->getHandle()}));
  mesh->getMaterial().set(render::scene::RenderMode::Full, m_texture);

  m_meshes.emplace_back(std::move(mesh));
}
} // namespace ui
