#pragma once

#include "switch.h"

namespace engine::objects
{
class UnderwaterSwitch final : public Switch
{
public:
  UnderwaterSwitch(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : Switch{engine, position}
  {
  }

  UnderwaterSwitch(const gsl::not_null<Engine*>& engine,
                   const gsl::not_null<const loader::file::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : Switch{engine, room, item, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
