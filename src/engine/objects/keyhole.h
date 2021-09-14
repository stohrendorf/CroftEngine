#pragma once

#include "modelobject.h"

namespace engine
{
struct CollisionInfo;
}

namespace engine::world
{
struct SkeletalModelType;
}

namespace engine::objects
{
class KeyHole final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(KeyHole, false)

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
