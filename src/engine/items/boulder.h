#pragma once

#include <loader/item.h>
#include "itemnode.h"

namespace engine
{
namespace items
{
class RollingBall final : public ModelItemNode
{
public:
    RollingBall(const gsl::not_null<level::Level*>& level,
                const gsl::not_null<const loader::Room*>& room,
                const loader::Item& item,
                const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, room, item, true, animatedModel )
            , m_position{room, item.position}
    {
    }

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

private:
    core::RoomBoundPosition m_position;
};
}
}
