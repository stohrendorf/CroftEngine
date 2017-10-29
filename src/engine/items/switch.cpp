#include "switch.h"

#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
namespace items
{
void Switch::onInteract(LaraNode& lara)
{
    if( !getLevel().m_inputHandler->getInputState().action )
    {
        return;
    }

    if( lara.getHandStatus() != 0 )
    {
        return;
    }

    if( lara.m_state.falling )
    {
        return;
    }

    if( m_triggerState != engine::items::TriggerState::Disabled )
    {
        return;
    }

    if( lara.getCurrentAnimState() != loader::LaraStateId::Stop )
    {
        return;
    }

    static const InteractionLimits limits{
        core::BoundingBox{{-200, 0, 312},
                          {+200, 0, 512}},
        {-10_deg, -30_deg, -10_deg},
        {+10_deg, +30_deg, +10_deg}
    };

    if( !limits.canInteract(*this, lara) )
    {
        return;
    }

    lara.m_state.rotation.Y = m_state.rotation.Y;

    if( getCurrentState() == 1 )
    {
        BOOST_LOG_TRIVIAL(debug) << "Switch " << getNode()->getId() << ": pull down";
        do
        {
            lara.setTargetState(loader::LaraStateId::SwitchDown);
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown );
        lara.setTargetState(loader::LaraStateId::Stop);
        m_state.goal_anim_state = 0;
        lara.setHandStatus(1);
    }
    else
    {
        if( getCurrentState() != 0 )
        {
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "Switch " << getNode()->getId() << ": pull up";
        do
        {
            lara.setTargetState(loader::LaraStateId::SwitchUp);
            lara.updateImpl();
        } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp );
        lara.setTargetState(loader::LaraStateId::Stop);
        m_state.goal_anim_state = 1;
        lara.setHandStatus(1);
    }

    m_triggerState = engine::items::TriggerState::Enabled;

    activate();
    ModelItemNode::update();
}
}
}
