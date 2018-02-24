#include "switch.h"

#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
namespace items
{
void Switch::collide(LaraNode& other, CollisionInfo& collisionInfo)
{
    if( !getLevel().m_inputHandler->getInputState().action )
    {
        return;
    }

    if( other.getHandStatus() != HandStatus::None )
    {
        return;
    }

    if( other.m_state.falling )
    {
        return;
    }

    if( m_state.triggerState != engine::items::TriggerState::Inactive )
    {
        return;
    }

    if( other.getCurrentAnimState() != loader::LaraStateId::Stop )
    {
        return;
    }

    static const InteractionLimits limits{
            core::BoundingBox{{-200, 0, 312},
                              {+200, 0, 512}},
            {-10_deg, -30_deg, -10_deg},
            {+10_deg, +30_deg, +10_deg}
    };

    if( !limits.canInteract( *this, other ) )
    {
        return;
    }

    other.m_state.rotation.Y = m_state.rotation.Y;

    if( m_state.current_anim_state == 1 )
    {
        BOOST_LOG_TRIVIAL( debug ) << "Switch " << getNode()->getId() << ": pull down";
        do
        {
            other.setTargetState( loader::LaraStateId::SwitchDown );
            other.updateImpl();
        } while( other.getCurrentAnimState() != loader::LaraStateId::SwitchDown );
        other.setTargetState( loader::LaraStateId::Stop );
        m_state.goal_anim_state = 0;
        other.setHandStatus( HandStatus::Grabbing );
    }
    else
    {
        if( m_state.current_anim_state != 0 )
        {
            return;
        }

        BOOST_LOG_TRIVIAL( debug ) << "Switch " << getNode()->getId() << ": pull up";
        do
        {
            other.setTargetState( loader::LaraStateId::SwitchUp );
            other.updateImpl();
        } while( other.getCurrentAnimState() != loader::LaraStateId::SwitchUp );
        other.setTargetState( loader::LaraStateId::Stop );
        m_state.goal_anim_state = 1;
        other.setHandStatus( HandStatus::Grabbing );
    }

    m_state.triggerState = engine::items::TriggerState::Active;

    activate();
    ModelItemNode::update();
}
}
}
