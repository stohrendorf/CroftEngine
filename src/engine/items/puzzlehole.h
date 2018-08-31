#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class PuzzleHole final : public ModelItemNode
{
public:
    PuzzleHole(const gsl::not_null<level::Level*>& level,
            const std::string& name,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, false, animatedModel )
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
