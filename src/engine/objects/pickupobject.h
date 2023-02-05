#pragma once

#include "serialization/serialization_fwd.h"
#include "spriteobject.h"

namespace engine
{
struct CollisionInfo;
}

namespace engine
{
struct Location;
}

namespace engine::world
{
class World;
struct Room;
struct Sprite;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace engine::objects
{
class PickupObject final : public SpriteObject
{
public:
  SPRITEOBJECT_DEFAULT_CONSTRUCTORS(PickupObject, true, true)

  void collide(CollisionInfo& collisionInfo) override;

  void serialize(const serialization::Serializer<world::World>& ser) const override;
  void deserialize(const serialization::Deserializer<world::World>& ser) override;
};
} // namespace engine::objects
