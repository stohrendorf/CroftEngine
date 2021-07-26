#pragma once

#include "modelobject.h"

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
