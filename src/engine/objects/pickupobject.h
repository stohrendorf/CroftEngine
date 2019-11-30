#pragma once

#include "spriteobject.h"

namespace engine::objects
{
class PickupObject final : public SpriteObject
{
public:
  PickupObject(const gsl::not_null<Engine*>& engine,
               const core::RoomBoundPosition& position,
               std::string name,
               const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteObject{engine, position, std::move(name), material}
  {
  }

  PickupObject(const gsl::not_null<Engine*>& engine,
               const std::string& name,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::Sprite*>& sprite,
               gsl::not_null<std::shared_ptr<render::scene::Material>> material)
      : SpriteObject{engine, name, room, item, true, sprite, std::move(material)}
  {
  }

  void update() override
  {
    updateLighting();
  }

  void collide(CollisionInfo& collisionInfo) override;
};
} // namespace engine
