#pragma once

#include "spriteobject.h"

namespace engine
{
struct CollisionInfo;
}

namespace engine::world
{
class World;
struct Sprite;
} // namespace engine::world

namespace engine::objects
{
class PickupObject final : public SpriteObject
{
public:
  SPRITEOBJECT_DEFAULT_CONSTRUCTORS(PickupObject, true)

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
