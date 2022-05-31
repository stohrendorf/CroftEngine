#pragma once

#include "aiagent.h"

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
class Raptor final : public AIAgent
{
public:
  Raptor(const gsl::not_null<world::World*>& world, const Location& location)
      : AIAgent{world, location}
  {
  }

  Raptor(const std::string& name,
         const gsl::not_null<world::World*>& world,
         const gsl::not_null<const world::Room*>& room,
         const loader::file::Item& item,
         const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;
  void serialize(const serialization::Serializer<world::World>& ser) override;
};
} // namespace engine::objects
