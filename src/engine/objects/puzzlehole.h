#pragma once

#include "modelobject.h"

namespace engine::objects
{
class PuzzleHole final : public ModelObject
{
public:
  PuzzleHole(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position);

  PuzzleHole(const gsl::not_null<World*>& world,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

private:
  void swapPuzzleState();
  void initMesh();
};
} // namespace engine::objects
