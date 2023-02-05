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
class Bear final : public AIAgent
{
public:
  AIAGENT_DEFAULT_CONSTRUCTORS(Bear)

  void update() override;

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;

private:
  bool m_hurt = false;
};
} // namespace engine::objects
