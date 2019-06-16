#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class PuzzleHole final : public ModelItemNode
{
public:
    PuzzleHole(const gsl::not_null<Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, false, animatedModel}
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
} // namespace items
} // namespace engine
