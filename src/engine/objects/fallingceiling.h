#pragma once

#include "modelobject.h"

namespace engine::objects
{
class FallingCeiling final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(FallingCeiling, true)

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
