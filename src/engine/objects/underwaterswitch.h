#pragma once

#include "switch.h"

namespace engine::objects
{
class UnderwaterSwitch final : public Switch
{
public:
  UnderwaterSwitch(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : Switch{world, position}
  {
  }

  UnderwaterSwitch(const gsl::not_null<World*>& world,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : Switch{world, room, item, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
