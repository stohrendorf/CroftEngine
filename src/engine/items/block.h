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
          const gsl::not_null<const loader::Room*>& room,
          const loader::Item& item,
          const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
        if( m_state.triggerState != TriggerState::Invisible )
            loader::Room::patchHeightsForBlock( *this, -loader::SectorSize );
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

    void update() override;

    void load(const YAML::Node& n) override;

private:
    bool isOnFloor(int height) const;

    bool canPushBlock(int height, core::Axis axis) const;

    bool canPullBlock(int height, core::Axis axis) const;
};
}
}
