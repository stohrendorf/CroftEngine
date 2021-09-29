#pragma once

#include "aiagent.h"
#include "core/angle.h"
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
class Natla final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Natla)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_attemptToFly = false;
  core::Frame m_flyTime = 0_frame;
  core::Angle m_pitchDelta = 0_deg;
};
} // namespace engine::objects
