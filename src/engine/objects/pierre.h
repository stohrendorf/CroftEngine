#pragma once

#include "aiagent.h"
#include "core/units.h"
#include "serialization/serialization_fwd.h"

// IWYU pragma: no_forward_declare serialization::Serializer

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
class Pierre final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Pierre)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  core::Frame m_fleeTime = 0_frame;
};
} // namespace engine::objects
