#include "util.h"

#include "loader/file/color.h"
#include "render/scene/material.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/rendercontext.h"

#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>

namespace ui
{
namespace
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const glm::vec2& topLeft,
                                                               const glm::vec2& bottomRight,
                                                               const BoxGouraud& colors,
                                                               const std::shared_ptr<render::scene::Material>& material)
{
  struct Vertex
  {
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 uv{0};
    const glm::int32_t texIndex{-1};
  };

  static const auto toColor = [](const gl::SRGBA8& c) { return glm::vec4{c.channels} / 255.0f; };

  const std::array<Vertex, 4> vertices{Vertex{{topLeft.x, topLeft.y}, toColor(colors.topLeft)},
                                       Vertex{{topLeft.x, bottomRight.y}, toColor(colors.bottomLeft)},
                                       Vertex{{bottomRight.x, bottomRight.y}, toColor(colors.bottomRight)},
                                       Vertex{{bottomRight.x, topLeft.y}, toColor(colors.topRight)}};

  static const gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                               {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv},
                                               {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &Vertex::texIndex},
                                               {VERTEX_ATTRIBUTE_COLOR_NAME, &Vertex::color}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto mesh
    = std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
      indexBuffer, vertexBuffer, std::vector{&material->getShaderProgram()->getHandle()}));
  mesh->getMaterial().set(render::scene::RenderMode::Full, material);
  return mesh;
}

gsl::not_null<std::shared_ptr<render::scene::Mesh>> createQuad(const glm::vec2& a,
                                                               const glm::vec2& dxy,
                                                               const gl::SRGBA8& color,
                                                               const std::shared_ptr<render::scene::Material>& material)
{
  struct Vertex
  {
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 uv{0};
    const glm::int32_t texIndex{-1};
  };

  static const auto toColor = [](const gl::SRGBA8& c) { return glm::vec4{c.channels} / 255.0f; };

  const auto b = a + dxy;
  const std::array<Vertex, 4> vertices{Vertex{{a.x, a.y}, toColor(color)},
                                       Vertex{{a.x, b.y}, toColor(color)},
                                       Vertex{{b.x, b.y}, toColor(color)},
                                       Vertex{{b.x, a.y}, toColor(color)}};

  static const gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos},
                                               {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &Vertex::uv},
                                               {VERTEX_ATTRIBUTE_TEXINDEX_NAME, &Vertex::texIndex},
                                               {VERTEX_ATTRIBUTE_COLOR_NAME, &Vertex::color}};

  auto vertexBuffer = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vertexBuffer->setData(&vertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto mesh
    = std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
      indexBuffer, vertexBuffer, std::vector{&material->getShaderProgram()->getHandle()}));
  mesh->getMaterial().set(render::scene::RenderMode::Full, material);
  return mesh;
}
} // namespace

void drawOutlineBox(const std::shared_ptr<render::scene::Material>& material,
                    const glm::ivec2& xy,
                    const glm::ivec2& size,
                    const loader::file::Palette& palette)
{
  const auto color1 = palette.colors[15].toTextureColor();
  const auto color2 = palette.colors[31].toTextureColor();

  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};

  // top
  createQuad(xy - glm::ivec2{0, 1}, {size.x + 1, 1}, color1, material)->render(ctx);
  createQuad(xy, {size.x, 1}, color2, material)->render(ctx);
  //right
  createQuad(xy + glm::ivec2{size.x, -1}, {1, size.y + 1}, color1, material)->render(ctx);
  createQuad(xy + glm::ivec2{size.x + 1, -1}, {1, size.y + 2}, color2, material)->render(ctx);
  // bottom
  createQuad(xy + size, {-size.x, 1}, color1, material)->render(ctx);
  createQuad(xy + size + glm::ivec2{0, 1}, {-size.x - 1, 1}, color2, material)->render(ctx);
  // left
  createQuad(xy + glm::ivec2{-1, size.y}, {1, -size.y - 1}, color1, material)->render(ctx);
  createQuad(xy + glm::ivec2{0, size.y}, {1, -size.y}, color2, material)->render(ctx);
}

void drawBox(const std::shared_ptr<render::scene::Material>& material,
             const glm::ivec2& xy,
             const glm::ivec2& size,
             const BoxGouraud& gouraud)
{
  Expects(size.x >= 0 && size.y >= 0);

  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};
  createQuad(xy, xy + size, gouraud, material)->render(ctx);
}

void drawBox(const std::shared_ptr<render::scene::Material>& material,
             const glm::ivec2& xy,
             const glm::ivec2& size,
             const gl::SRGBA8& color)
{
  Expects(size.x >= 0 && size.y >= 0);

  render::scene::RenderContext ctx{render::scene::RenderMode::Full, std::nullopt};
  createQuad(xy, size, color, material)->render(ctx);
}
} // namespace ui
