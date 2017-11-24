#include "underwaterswitch.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void UnderwaterSwitch::collide(LaraNode& other, CollisionInfo& collisionInfo)
{
    if( !getLevel().m_inputHandler->getInputState().action )
    {
        return;
    }

    if( m_state.triggerState != engine::items::TriggerState::Disabled )
    {
        return;
    }

    if( !other.isDiving() )
    {
        return;
    }

    if( other.getCurrentAnimState() != LaraStateId::UnderwaterStop )
    {
        return;
    }

    static const InteractionLimits limits{
        core::BoundingBox{{-1024, -1024, -1024},
                          {1024,  1024,  512}},
        {-80_deg, -80_deg, -80_deg},
        {+80_deg, +80_deg, +80_deg}
    };

    if( !limits.canInteract(*this, other) )
    {
        return;
    }

    if( m_state.current_anim_state != 0 && m_state.current_anim_state != 1 )
    {
        return;
    }

    static const glm::vec3 alignSpeed{0, 0, -108.0f};
    if( !other.alignTransform(alignSpeed, *this) )
    {
        return;
    }

    other.m_state.fallspeed = 0;
    do
    {
        other.setTargetState(LaraStateId::SwitchDown);
        other.updateImpl();
    } while( other.getCurrentAnimState() != LaraStateId::SwitchDown );
    other.setTargetState(LaraStateId::UnderwaterStop);
    other.setHandStatus(1);
    m_state.triggerState = engine::items::TriggerState::Enabled;

    if( m_state.current_anim_state == 1 )
    {
        m_state.goal_anim_state = 0;
    }
    else
    {
        m_state.goal_anim_state = 1;
    }

    activate();
    ModelItemNode::update();
}
}
}
