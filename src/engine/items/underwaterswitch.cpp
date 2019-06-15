#include "underwaterswitch.h"

#include "engine/laranode.h"
#include "hid/inputhandler.h"

namespace engine
{
namespace items
{
void UnderwaterSwitch::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    if( !getEngine().getInputHandler().getInputState().action )
        return;

    if( m_state.triggerState != TriggerState::Inactive )
        return;

    if( !lara.isDiving() )
        return;

    if( lara.getCurrentAnimState() != LaraStateId::UnderwaterStop )
        return;

    static const InteractionLimits limits{
        core::BoundingBox{ { -1024_len, -1024_len, -1024_len },
                           { 1024_len,  1024_len,  512_len }
        },
        { -80_deg, -80_deg, -80_deg },
        { +80_deg, +80_deg, +80_deg }
    };

    if( !limits.canInteract( m_state, lara.m_state ) )
        return;

    if( m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as )
        return;

    static const core::TRVec alignSpeed{ 0_len, 0_len, 108_len };
    if( !lara.alignTransform( alignSpeed, *this ) )
        return;

    lara.m_state.fallspeed = 0_spd;
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
