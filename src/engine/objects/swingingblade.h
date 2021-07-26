#pragma once

#include "modelobject.h"

namespace engine::objects
{
class SwingingBlade final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(SwingingBlade, true)

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
