#pragma once

#include "modelobject.h"

namespace engine::objects
{
class PuzzleHole final : public ModelObject
{
public:
  PuzzleHole(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : ModelObject{world, position}
  {
  }

  PuzzleHole(const gsl::not_null<World*>& world,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{world, room, item, false, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
