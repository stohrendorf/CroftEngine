#pragma once

#include "switch.h"

namespace engine
{
namespace items
{
class UnderwaterSwitch final : public Switch
{
public:
    UnderwaterSwitch(const gsl::not_null<level::Level*>& level,
                     const std::string& name,
                     const gsl::not_null<const loader::Room*>& room,
                     const loader::Item& item,
                     const loader::SkeletalModelType& animatedModel)
            : Switch( level, name, room, item, animatedModel )
    {
        m_state.collidable = true;
        m_state.is_hit = true;
    }

    void collide(LaraNode& other, CollisionInfo& collisionInfo) override;

    void update() override
    {
        if( !m_isActive )
            return;

        m_state.activationState.fullyActivate();

        ModelItemNode::update();
    }
};
}
}
