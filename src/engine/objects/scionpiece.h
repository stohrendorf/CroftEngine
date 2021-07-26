#pragma once

#include "modelobject.h"
#include "spriteobject.h"

namespace engine::objects
{
class ScionPiece final : public SpriteObject
{
public:
  SPRITEOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece, true)

  void collide(CollisionInfo& collisionInfo) override;
};

class ScionPiece3 final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece3, true)

  void update() override;

private:
  core::Frame m_deadTime = 0_frame;
};

class ScionPiece4 final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece4, true)

  void collide(CollisionInfo& info) override;
};

class ScionHolder final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionHolder, true)

  void collide(CollisionInfo& info) override;
};
} // namespace engine::objects
