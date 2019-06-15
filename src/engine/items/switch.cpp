#include "switch.h"

#include "engine/laranode.h"
#include "hid/inputhandler.h"

namespace engine
{
namespace items
{
void Switch::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    if( !getEngine().getInputHandler().getInputState().action )
        return;

    if( lara.getHandStatus() != HandStatus::None )
        return;

    if( lara.m_state.falling )
        return;

    if( m_state.triggerState != TriggerState::Inactive )
        return;

    if( lara.getCurrentAnimState() != LaraStateId::Stop )
        return;

    static const InteractionLimits limits{
        core::BoundingBox{ { -200_len, 0_len, 312_len },
                           { +200_len, 0_len, 512_len }
        },
        { -10_deg, -30_deg, -10_deg },
        { +10_deg, +30_deg, +10_deg }
    };

    if( !limits.canInteract( m_state, lara.m_state ) )
        return;

    lara.m_state.rotation.Y = m_state.rotation.Y;

    if( m_state.current_anim_state == 1_as )
    {
        do
        {
            lara.setGoalAnimState( LaraStateId::SwitchDown );
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != LaraStateId::SwitchDown );
        lara.setGoalAnimState( LaraStateId::Stop );
        m_state.goal_anim_state = 0_as;
        lara.setHandStatus( HandStatus::Grabbing );
    }
    else
    {
        if( m_state.current_anim_state != 0_as )
            return;

        do
        {
            lara.setGoalAnimState( LaraStateId::SwitchUp );
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != LaraStateId::SwitchUp );
        lara.setGoalAnimState( LaraStateId::Stop );
        m_state.goal_anim_state = 1_as;
        lara.setHandStatus( HandStatus::Grabbing );
    }

    m_state.triggerState = TriggerState::Active;

    activate();
    ModelItemNode::update();
}
}
}
