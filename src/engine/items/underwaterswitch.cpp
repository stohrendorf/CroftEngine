#include "underwaterswitch.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void UnderwaterSwitch::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    if( !getLevel().m_inputHandler->getInputState().action )
        return;

    if( m_state.triggerState != TriggerState::Inactive )
        return;

    if( !lara.isDiving() )
        return;

    if( lara.getCurrentAnimState() != LaraStateId::UnderwaterStop )
        return;

    static const InteractionLimits limits{
            core::BoundingBox{{-1024, -1024, -1024},
                              {1024,  1024,  512}},
            {-80_deg, -80_deg, -80_deg},
            {+80_deg, +80_deg, +80_deg}
    };

    if( !limits.canInteract( m_state, lara.m_state ) )
        return;

    if( m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as )
        return;

    static const core::TRVec alignSpeed{0, 0, 108};
    if( !lara.alignTransform( alignSpeed, *this ) )
        return;

    lara.m_state.fallspeed = 0;
    do
    {
        lara.setGoalAnimState( LaraStateId::SwitchDown );
        lara.updateImpl();
    } while( lara.getCurrentAnimState() != LaraStateId::SwitchDown );
    lara.setGoalAnimState( LaraStateId::UnderwaterStop );
    lara.setHandStatus( HandStatus::Grabbing );
    m_state.triggerState = TriggerState::Active;

    if( m_state.current_anim_state == 1_as )
    {
        m_state.goal_anim_state = 0_as;
    }
    else
    {
        m_state.goal_anim_state = 1_as;
    }

    activate();
    ModelItemNode::update();
}
}
}
