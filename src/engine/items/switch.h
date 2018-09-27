#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Switch : public ModelItemNode
{
public:
    Switch(const gsl::not_null<level::Level*>& level,
           const gsl::not_null<const loader::Room*>& room,
           const loader::Item& item,
           const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

    void update() final
    {
        m_state.activationState.fullyActivate();
        if( !m_state.updateActivationTimeout() )
        {
            m_state.goal_anim_state = 1;
            m_state.timer = 0;
        }

        ModelItemNode::update();
    }
};
}
}
