#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class ScionPieceItem final : public SpriteItemNode
{
public:
    ScionPieceItem(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::Sprite& sprite,
                   const gsl::not_null<std::shared_ptr<gameplay::Material>>& material)
            : SpriteItemNode( level, name, room, item, true, sprite, material )
    {
    }

    void update() override
    {
        updateLighting();
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
