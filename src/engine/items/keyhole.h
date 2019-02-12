#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class KeyHole final : public ModelItemNode
{
public:
    KeyHole(const gsl::not_null<loader::file::level::Level*>& level,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const loader::file::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, false, animatedModel}
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
