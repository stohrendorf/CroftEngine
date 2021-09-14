#pragma once

#include "switch.h"

#include <gsl/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

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
class UnderwaterSwitch final : public Switch
{
public:
  UnderwaterSwitch(const gsl::not_null<world::World*>& world, const Location& location)
      : Switch{world, location}
  {
  }

  UnderwaterSwitch(const std::string& name,
                   const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : Switch{name, world, room, item, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
