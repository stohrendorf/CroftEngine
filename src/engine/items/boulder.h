#pragma once

#include "loader/file/item.h"
#include "itemnode.h"

namespace engine
{
namespace items
{
class RollingBall final : public ModelItemNode
{
public:
    RollingBall(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{ engine, room, item, true, animatedModel }
          , m_position{ room, item.position }
    {
    }

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

private:
    core::RoomBoundPosition m_position;
};
}
}
