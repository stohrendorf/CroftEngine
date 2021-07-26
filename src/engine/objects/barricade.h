#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Barricade final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Barricade, true)

  void update() override;

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
