#pragma once

#include "spriteobject.h"

namespace engine::objects
{
class PickupObject final : public SpriteObject
{
public:
  PickupObject(const gsl::not_null<world::World*>& world,
               const core::RoomBoundPosition& position,
               std::string name,
               const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteObject{world, position, std::move(name), material}
  {
  }

  PickupObject(const gsl::not_null<world::World*>& world,
               const std::string& name,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const world::Sprite*>& sprite,
               gsl::not_null<std::shared_ptr<render::scene::Material>> material)
      : SpriteObject{world, name, room, item, true, sprite, std::move(material)}
  {
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine::objects
