#pragma once

#include "engine/location.h"
#include "modelobject.h"

#include <gsl/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
}

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
class RollingBall final : public ModelObject
{
public:
  RollingBall(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
      , m_location{location}
  {
  }

  RollingBall(const std::string& name,
              const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;

private:
  Location m_location;
};
} // namespace engine::objects
