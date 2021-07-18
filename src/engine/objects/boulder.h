#pragma once

#include "loader/file/item.h"
#include "modelobject.h"

namespace engine::objects
{
class RollingBall final : public ModelObject
{
public:
  RollingBall(const gsl::not_null<world::World*>& world, const RoomBoundPosition& position)
      : ModelObject{world, position}
      , m_position{position}
  {
  }

  RollingBall(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
      , m_position{room, item.position}
  {
  }

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;

private:
  RoomBoundPosition m_position;
};
} // namespace engine::objects
