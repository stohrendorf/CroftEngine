#pragma once

#include "modelobject.h"

namespace engine::objects
{
class SwordOfDamocles final : public ModelObject
{
public:
  SwordOfDamocles(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  SwordOfDamocles(const gsl::not_null<world::World*>& world,
                  const gsl::not_null<const loader::file::Room*>& room,
                  const loader::file::Item& item,
                  const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
