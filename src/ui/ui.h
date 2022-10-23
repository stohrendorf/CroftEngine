#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <vector>

namespace render::material
{
class Material;
}

namespace engine::world
{
struct Sprite;
}

namespace ui
{
struct BoxGouraud;

class Ui final
{
public:
  struct UiVertex
  {
    glm::vec2 pos;
    glm::vec3 uv;
    glm::vec4 topLeft{0};
    glm::vec4 topRight{0};
    glm::vec4 bottomLeft{0};
    glm::vec4 bottomRight{0};
    glm::vec4 color{1, 1, 1, 1};

    static gslu::nn_shared<gl::VertexBuffer<Ui::UiVertex>> createVertexBuffer(gl::api::BufferUsage usage,
                                                                              const gsl::span<Ui::UiVertex>& data);
    static gslu::nn_shared<gl::ElementArrayBuffer<uint16_t>> createIndexBuffer(gl::api::BufferUsage usage,
                                                                               const gsl::span<uint16_t>& data);
  };

  explicit Ui(std::shared_ptr<render::material::Material> material,
              const std::array<gl::SRGBA8, 256>& palette,
              const glm::ivec2& size);

  void drawOutlineBox(const glm::ivec2& xy, const glm::ivec2& size, uint8_t alpha = 255);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const BoxGouraud& gouraud);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const gl::SRGBA8& color);
  void drawBox(const glm::ivec2& xy, const glm::ivec2& size, const size_t color)
  {
    drawBox(xy, size, m_palette.at(color));
  }

  void drawHLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color);
  void drawVLine(const glm::ivec2& xy, int length, const gl::SRGBA8& color);
  void draw(const engine::world::Sprite& sprite, const glm::ivec2& xy, float scale = 1, float alpha = 1);

  void render();

  [[nodiscard]] const auto& getSize() const
  {
    return m_size;
  }

private:
  const std::shared_ptr<render::material::Material> m_material;
  const std::array<gl::SRGBA8, 256> m_palette;
  const glm::ivec2 m_size;
  std::vector<UiVertex> m_vertices{};
};
} // namespace ui
