#pragma once

#include "aiagent.h"
#include "core/units.h"
#include "serialization/serialization_fwd.h"

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class Cowboy final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Cowboy)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  core::RenderFrame m_aimTime = 0_rframe;
};
} // namespace engine::objects
