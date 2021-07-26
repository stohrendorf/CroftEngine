#pragma once

#include "spriteobject.h"

namespace engine::objects
{
class PickupObject final : public SpriteObject
{
public:
  SPRITEOBJECT_DEFAULT_CONSTRUCTORS(PickupObject, true)

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
