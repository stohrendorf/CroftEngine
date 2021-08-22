#pragma once

#include "modelobject.h"

namespace engine::objects
{
class PuzzleHole final : public ModelObject
{
public:
  PuzzleHole(const gsl::not_null<world::World*>& world, const Location& location);

  PuzzleHole(const std::string& name,
             const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

private:
  void swapPuzzleState();
  void initMesh();
};
} // namespace engine::objects
