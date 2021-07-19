#pragma once

#include "switch.h"

namespace engine::objects
{
class UnderwaterSwitch final : public Switch
{
public:
  UnderwaterSwitch(const gsl::not_null<world::World*>& world, const RoomBoundPosition& location)
      : Switch{world, location}
  {
  }

  UnderwaterSwitch(const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : Switch{world, room, item, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
