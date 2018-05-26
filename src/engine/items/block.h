#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Block final : public ModelItemNode
{
public:
    Block(const gsl::not_null<level::Level*>& level,
          const std::string& name,
          const gsl::not_null<const loader::Room*>& room,
          const loader::Item& item,
          const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
        if( m_state.triggerState != TriggerState::Invisible )
            loader::Room::patchHeightsForBlock( *this, -loader::SectorSize );
        m_state.is_hit = true;
        m_state.collidable = true;
    }

    void collide(LaraNode& other, CollisionInfo& collisionInfo) override;

    void update() override;

private:
    bool isOnFloor(int height) const;

    bool canPushBlock(int height, core::Axis axis) const;

    bool canPullBlock(int height, core::Axis axis) const;
};
}
}
