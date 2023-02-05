#pragma once

#include "core/units.h"
#include "modelobject.h"
#include "spriteobject.h"

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
struct Sprite;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class ScionPiece final : public SpriteObject
{
public:
  SPRITEOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece, true, false)

  void collide(CollisionInfo& collisionInfo) override;
  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;
};

class ScionPiece3 final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece3, true, false)

  void update() override;

private:
  core::Frame m_deadTime = 0_frame;
};

class ScionPiece4 final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionPiece4, true, false)

  void collide(CollisionInfo& info) override;
};

class ScionHolder final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(ScionHolder, true, false)

  void collide(CollisionInfo& info) override;
};
} // namespace engine::objects
