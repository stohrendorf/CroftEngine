#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class KeyHole final : public ModelItemNode
{
public:
    KeyHole(const gsl::not_null<level::Level*>& level,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, false, animatedModel}
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
