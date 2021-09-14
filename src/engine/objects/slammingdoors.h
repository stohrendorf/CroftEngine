#pragma once

#include "modelobject.h"

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
class SlammingDoors final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(SlammingDoors, true)

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
