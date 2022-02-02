#include "ui.h"

#include "boxgouraud.h"
#include "core/id.h"
#include "engine/world/sprite.h"
#include "render/scene/material.h"
#include "render/scene/materialgroup.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "render/scene/shaderprogram.h"

#include <algorithm>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/debuggroup.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <glm/common.hpp>
#include <glm/mat4x4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <optional>
#include <string>
#include <utility>

namespace ui
{
namespace
{
void createQuad(std::vector<Ui::UiVertex>& vertices,
                const glm::vec2& topLeft,
                const glm::vec2& bottomRight,
                const BoxGouraud& colors)
{
  static const auto toColor = [](const gl::SRGBA8& c)
  {
    return glm::vec4{c.channels} / 255.0f;
  };

  vertices.emplace_back(Ui::UiVertex{
    {topLeft.x, topLeft.y},
    {0, 0, -1},
    toColor(colors.topLeft),
    toColor(colors.topRight),
    toColor(colors.bottomLeft),
    toColor(colors.bottomRight),
  });

  vertices.emplace_back(Ui::UiVertex{
    {topLeft.x, bottomRight.y},
    {0, 1, -1},
    toColor(colors.topLeft),
    toColor(colors.topRight),
    toColor(colors.bottomLeft),
    toColor(colors.bottomRight),
  });
  vertices.emplace_back(Ui::UiVertex{
    {bottomRight.x, bottomRight.y},
    {1, 1, -1},
    toColor(colors.topLeft),
    toColor(colors.topRight),
    toColor(colors.bottomLeft),
    toColor(colors.bottomRight),
  });
  vertices.emplace_back(Ui::UiVertex{
    {bottomRight.x, topLeft.y},
    {1, 0, -1},
    toColor(colors.topLeft),
    toColor(colors.topRight),
    toColor(colors.bottomLeft),
    toColor(colors.bottomRight),
  });
}

void createQuad(std::vector<Ui::UiVertex>& vertices, const glm::vec2& a, const glm::vec2& dxy, const gl::SRGBA8& color)
{
  const auto glColor = glm::vec4{color.channels} / 255.0f;
  const auto b = a + dxy;

  vertices.emplace_back(Ui::UiVertex{{a.x, a.y}, {0, 0, -1}, glColor, glColor, glColor, glColor});
  vertices.emplace_back(Ui::UiVertex{{a.x, b.y}, {0, 1, -1}, glColor, glColor, glColor, glColor});
  vertices.emplace_back(Ui::UiVertex{{b.x, b.y}, {1, 1, -1}, glColor, glColor, glColor, glColor});
  vertices.emplace_back(Ui::UiVertex{{b.x, a.y}, {1, 0, -1}, glColor, glColor, glColor, glColor});
}

void createHLine(std::vector<Ui::UiVertex>& vertices, const glm::vec2& a, int length, const gl::SRGBA8& color)
{
  return createQuad(vertices, a, glm::vec2{length, 1}, color);
}

void createVLine(std::vector<Ui::UiVertex>& vertices, const glm::vec2& a, int length, const gl::SRGBA8& color)
{
  return createQuad(vertices, a, glm::vec2{1, length}, color);
}
} // namespace

gslu::nn_shared<gl::VertexBuffer<Ui::UiVertex>> Ui::UiVertex::createVertexBuffer()
{
  static const gl::VertexLayout<UiVertex> layout{
    {VERTEX_ATTRIBUTE_POSITION_NAME, &UiVertex::pos},
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &UiVertex::uv},
    {VERTEX_ATTRIBUTE_COLOR_TOP_LEFT_NAME, &UiVertex::topLeft},
    {VERTEX_ATTRIBUTE_COLOR_TOP_RIGHT_NAME, &UiVertex::topRight},
    {VERTEX_ATTRIBUTE_COLOR_BOTTOM_LEFT_NAME, &UiVertex::bottomLeft},
    {VERTEX_ATTRIBUTE_COLOR_BOTTOM_RIGHT_NAME, &UiVertex::bottomRight},
    {VERTEX_ATTRIBUTE_COLOR_NAME, &UiVertex::color},
  };
  return gsl::make_shared<gl::VertexBuffer<UiVertex>>(layout, "ui-vbo");
}

gslu::nn_shared<gl::ElementArrayBuffer<uint16_t>> Ui::UiVertex::createIndexBuffer()
{
  return gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>("ui-indices");
}

Ui::Ui(std::shared_ptr<render::scene::Material> material,
       const std::array<gl::SRGBA8, 256>& palette,
       const glm::ivec2& size)
    : m_material{std::move(material)}
    , m_palette{palette}
    , m_size{size}
{
}

void Ui::drawHLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color)
{
  createHLine(m_vertices, xy, length + glm::sign(length), color);
}

void Ui::drawVLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color)
{
  createVLine(m_vertices, xy, length + glm::sign(length), color);
}

void Ui::drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size, uint8_t alpha)
{
  auto color1 = m_palette[15];
  color1.channels[3] = alpha;
  auto color2 = m_palette[31];
  color2.channels[3] = alpha;

  // top
  drawHLine(xy - glm::ivec2{0, 1}, size.x, color1);
  drawHLine(xy, size.x, color2);
  //right
  drawVLine(xy + glm::ivec2{size.x, -1}, size.y + 1, color1);
  drawVLine(xy + glm::ivec2{size.x + 1, -1}, size.y + 2, color2);
  // bottom
  drawHLine(xy + size, -size.x + 1, color1);
  drawHLine(xy + size + glm::ivec2{1, 1}, -size.x - 1, color2);
  // left
  drawVLine(xy + glm::ivec2{-1, size.y + 1}, -size.y - 1, color1);
  drawVLine(xy + glm::ivec2{0, size.y}, -size.y + 1, color2);
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud)
{
  createQuad(m_vertices, xy, xy + size, gouraud);
}

void Ui::drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color)
{
  createQuad(m_vertices, xy, size, color);
}

void Ui::render()
{
  SOGLB_DEBUGGROUP("ui");

  const auto indexBuffer = UiVertex::createIndexBuffer();
  const auto vbo = UiVertex::createVertexBuffer();
  {
    // rebuild vao and vbo
    Expects(m_vertices.size() % 4 == 0);

    std::vector<uint16_t> indices;
    static const std::array<uint16_t, 6> localIndices{0, 1, 2, 0, 2, 3};
    indices.reserve(m_vertices.size() / 4 * localIndices.size());
    for(size_t i = 0; i < m_vertices.size(); i += 4)
    {
      for(auto localIndex : localIndices)
        indices.emplace_back(gsl::narrow_cast<uint16_t>(i + localIndex));
    }

    indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);
    vbo->setData(m_vertices, gl::api::BufferUsage::StaticDraw);
  }

  const auto vao = gsl::make_shared<gl::VertexArray<uint16_t, UiVertex>>(
    indexBuffer, std::tuple{vbo}, std::vector{&m_material->getShaderProgram()->getHandle()}, "ui-vao");
  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, UiVertex>>(vao);
  mesh->getMaterialGroup().set(render::scene::RenderMode::Full, m_material);
  mesh->getRenderState().setViewport(m_size);
  mesh->getRenderState().setBlend(0, true);
  mesh->getRenderState().setBlendFactors(0,
                                         gl::api::BlendingFactor::SrcAlpha,
                                         gl::api::BlendingFactor::One,
                                         gl::api::BlendingFactor::OneMinusSrcAlpha,
                                         gl::api::BlendingFactor::One);
  mesh->getRenderState().setDepthTest(false);
  mesh->getRenderState().setDepthWrite(false);

  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};
  mesh->render(nullptr, ctx);

  m_vertices.clear();
}

void Ui::draw(const engine::world::Sprite& sprite, const glm::ivec2& xy, float scale, float alpha)
{
  const auto a = glm::ivec2{glm::vec2{sprite.render0} * scale} + xy;
  const auto b = glm::ivec2{glm::vec2{sprite.render1} * scale} + xy;
  const auto ta = sprite.uv0;
  const auto tb = sprite.uv1;
  m_vertices.emplace_back(UiVertex{{a.x, a.y},
                                   {ta.x, ta.y, sprite.textureId.get()},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   {1, 1, 1, alpha}});
  m_vertices.emplace_back(UiVertex{{a.x, b.y},
                                   {ta.x, tb.y, sprite.textureId.get()},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   {1, 1, 1, alpha}});
  m_vertices.emplace_back(UiVertex{{b.x, b.y},
                                   {tb.x, tb.y, sprite.textureId.get()},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   {1, 1, 1, alpha}});
  m_vertices.emplace_back(UiVertex{{b.x, a.y},
                                   {tb.x, ta.y, sprite.textureId.get()},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   glm::vec4{0},
                                   {1, 1, 1, alpha}});
}
} // namespace ui
