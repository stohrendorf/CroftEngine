#pragma once

#include "render/scene/node.h"

#include <cstdint>
#include <gl/buffer.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace engine::world
{
class World;
}

namespace render::scene
{
class Mesh;
}

namespace engine::ghosting
{
struct GhostFrame;

class GhostModel final : public render::scene::Node
{
public:
  GhostModel()
      : render::scene::Node{"ghost"}
  {
    setColor(gl::SRGB8{51, 51, 204});
  }

  void apply(const engine::world::World& world, const GhostFrame& frame);

  [[nodiscard]] const auto& getMeshMatricesBuffer() const
  {
    if(m_meshMatricesBuffer == nullptr)
      m_meshMatricesBuffer = std::make_unique<gl::ShaderStorageBuffer<glm::mat4>>(
        "mesh-matrices-ssb", gl::api::BufferUsage::DynamicDraw, gsl::span<glm::mat4>{});
    return *m_meshMatricesBuffer;
  }

  [[nodiscard]] auto getRoomId() const
  {
    return m_roomId;
  }

  void setColor(const gl::SRGB8& color)
  {
    bind("u_color",
         [color = glm::vec3{color.channels}
                  / 255.0f](const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
         {
           uniform.set(color);
         });
  }

private:
  mutable std::unique_ptr<gl::ShaderStorageBuffer<glm::mat4>> m_meshMatricesBuffer;
  uint32_t m_roomId = 0;
};
} // namespace engine::ghosting
