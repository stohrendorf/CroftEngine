#pragma once

#include "modelobject.h"

namespace engine::objects
{
class TeethSpikes final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(TeethSpikes, true)

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
