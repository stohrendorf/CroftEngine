#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TeethSpikes final : public ModelObject
{
public:
  TeethSpikes(const gsl::not_null<world::World*>& world, const RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  TeethSpikes(const gsl::not_null<world::World*>& world,
              const gsl::not_null<const world::Room*>& room,
              const loader::file::Item& item,
              const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
