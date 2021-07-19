#pragma once

#include "loader/file/item.h"
#include "modelobject.h"

namespace engine::objects
{
class RollingBall final : public ModelObject
{
public:
  RollingBall(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : ModelObject{world, location}
      , m_location{location}
  {
  }

  RollingBall(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
      , m_location{room, item.position}
  {
  }

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;

private:
  RoomBoundPosition m_location;
};
} // namespace engine::objects
