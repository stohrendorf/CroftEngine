#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Barricade final : public ModelItemNode
{
public:
    Barricade(const gsl::not_null<level::Level*>& level,
              const gsl::not_null<const loader::Room*>& room,
              const loader::Item& item,
              const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
