#pragma once

#include "render/scene/node.h"

#include <cstdint>
#include <gl/buffer.h>
#include <glm/mat4x4.hpp>
#include <string>

namespace engine::world
{
class World;
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

private:
  mutable std::unique_ptr<gl::ShaderStorageBuffer<glm::mat4>> m_meshMatricesBuffer;
  uint32_t m_roomId = 0;
};
} // namespace engine::ghosting
