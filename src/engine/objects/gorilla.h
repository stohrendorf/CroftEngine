#pragma once

#include "aiagent.h"
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
class Gorilla final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Gorilla)

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  bool m_wantAttack = false;
  bool m_turnedRight = false;
  bool m_turnedLeft = false;
};
} // namespace engine::objects
