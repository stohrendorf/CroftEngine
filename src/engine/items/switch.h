#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Switch : public ModelItemNode
{
public:
    Switch(const gsl::not_null<Engine*>& engine,
           const gsl::not_null<const loader::file::Room*>& room,
           const loader::file::Item& item,
           const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{ engine, room, item, true, animatedModel }
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

    void update() final
    {
        m_state.activationState.fullyActivate();
        if( !m_state.updateActivationTimeout() )
        {
            // reset switch if a timeout is active
            m_state.goal_anim_state = 1_as;
            m_state.timer = 0_frame;
        }

        ModelItemNode::update();
    }
};
}
}
