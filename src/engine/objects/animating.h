#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Animating final : public ModelObject
{
public:
  Animating(const gsl::not_null<world::World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  Animating(const gsl::not_null<world::World*>& world,
            const gsl::not_null<const world::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, true, animatedModel}
  {
  }

  void update() override;
};
} // namespace engine::objects
