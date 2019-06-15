#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Block final : public ModelItemNode
{
public:
    Block(const gsl::not_null<Engine*>& engine,
          const gsl::not_null<const loader::file::Room*>& room,
          const loader::file::Item& item,
          const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{ engine, room, item, true, animatedModel }
    {
        if( m_state.triggerState != TriggerState::Invisible )
            loader::file::Room::patchHeightsForBlock( *this, -core::SectorSize );
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

    void update() override;

    void load(const YAML::Node& n) override;

private:
    bool isOnFloor(const core::Length& height) const;

    bool canPushBlock(const core::Length& height, core::Axis axis) const;

    bool canPullBlock(const core::Length& height, core::Axis axis) const;
};
}
}
