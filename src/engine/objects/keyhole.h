#pragma once

#include "modelobject.h"

namespace engine::objects
{
class KeyHole final : public ModelObject
{
public:
  KeyHole(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  KeyHole(const gsl::not_null<world::World*>& world,
          const gsl::not_null<const world::Room*>& room,
          const loader::file::Item& item,
          const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
