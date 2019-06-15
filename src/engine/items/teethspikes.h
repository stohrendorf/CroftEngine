#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class TeethSpikes final : public ModelItemNode
{
public:
    TeethSpikes(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{ engine, room, item, true, animatedModel }
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
