#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class ScionPieceItem final : public SpriteItemNode
{
public:
  ScionPieceItem(const gsl::not_null<Engine*>& engine,
                 const std::string& name,
                 const gsl::not_null<const loader::file::Room*>& room,
                 const loader::file::Item& item,
                 const loader::file::Sprite& sprite,
                 const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
      : SpriteItemNode{engine, name, room, item, true, sprite, material}
  {
  }

  void update() override
  {
    updateLighting();
  }

  void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
} // namespace items
} // namespace engine
