#pragma once

#include "modelobject.h"

#include <gsl-lite/gsl-lite.hpp>
#include <string>

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class PuzzleHole final : public ModelObject
{
public:
  PuzzleHole(const gsl_lite::not_null<world::World*>& world, const Location& location);

  PuzzleHole(const std::string& name,
             const gsl_lite::not_null<world::World*>& world,
             const gsl_lite::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel);

  void collide(CollisionInfo& collisionInfo) override;

  void updateLogic() override;

private:
  void swapPuzzleState();
  void initMesh();
};
} // namespace engine::objects