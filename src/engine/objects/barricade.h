#pragma once

#include "modelobject.h"

namespace engine
{
struct CollisionInfo;
}

namespace engine
{
struct Location;
}

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
class Barricade final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Barricade, true, false)

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
