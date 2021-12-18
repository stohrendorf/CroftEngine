#pragma once

#include "engine/skeletalmodelnode.h"
#include "engine/world/rendermeshdata.h"

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
  GhostModel() : render::scene::Node{"ghost"}{}

  void apply(const engine::world::World& world, const GhostFrame& frame);

  [[nodiscard]] const auto& getMeshMatricesBuffer() const
  {
    return m_meshMatricesBuffer;
  }

  [[nodiscard]] auto getRoomId() const
  {
    return m_roomId;
  }

private:
  mutable gl::ShaderStorageBuffer<glm::mat4> m_meshMatricesBuffer{"mesh-matrices-ssb"};
  uint32_t m_roomId = 0;
};
} // namespace engine::ghosting
