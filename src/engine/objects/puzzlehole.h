#pragma once

#include "modelobject.h"

namespace engine::objects
{
class PuzzleHole final : public ModelObject
{
public:
  PuzzleHole(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  PuzzleHole(const gsl::not_null<Engine*>& engine,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : ModelObject{engine, room, item, false, animatedModel}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine
