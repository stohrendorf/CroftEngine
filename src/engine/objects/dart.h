#pragma once

#include "modelobject.h"
#include "objectstate.h"

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
class Dart final : public ModelObject
{
public:
  Dart(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location, true}
  {
  }

  Dart(const std::string& name,
       const gsl::not_null<world::World*>& world,
       const gsl::not_null<const world::Room*>& room,
       const loader::file::Item& item,
       const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{name, world, room, item, true, animatedModel, true}
  {
    m_state.collidable = true;
  }

  void collide(CollisionInfo& info) override;

  void update() override;
};
} // namespace engine::objects
