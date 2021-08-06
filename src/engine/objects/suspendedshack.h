#pragma once

#include "engine/world/sector.h"
#include "modelobject.h"

namespace engine::objects
{
class SuspendedShack final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(SuspendedShack, true);

  void update() override;
  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
