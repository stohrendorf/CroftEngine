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
                return;

            if( lara.getHandStatus() != 0 )
                return;

            if( lara.isFalling() )
                return;

            if( m_triggerState != engine::items::TriggerState::Disabled )
                return;

            if( lara.getCurrentAnimState() != loader::LaraStateId::Stop )
                return;

            static const InteractionLimits limits{
                core::BoundingBox{{-200, 0, 312}, {+200, 0, 512}},
                    {-10_deg, -30_deg, -10_deg},
                    {+10_deg, +30_deg, +10_deg}
            };

            if( !limits.canInteract( *this, lara ) )
                return;

            lara.setYRotation( getRotation().Y );

            if( getNode()->getCurrentState() == 1 )
            {
                BOOST_LOG_TRIVIAL( debug ) << "Switch " << getNode()->getId() << ": pull down";
                do
                {
                    lara.setTargetState( loader::LaraStateId::SwitchDown );
                    lara.updateImpl();
                } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown );
                lara.setTargetState( loader::LaraStateId::Stop );
                getNode()->setTargetState( 0 );
                lara.setHandStatus( 1 );
            }
            else
            {
                if( getNode()->getCurrentState() != 0 )
                    return;

                BOOST_LOG_TRIVIAL( debug ) << "Switch " << getNode()->getId() << ": pull up";
                do
                {
                    lara.setTargetState( loader::LaraStateId::SwitchUp );
                    lara.updateImpl();
                } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp );
                lara.setTargetState( loader::LaraStateId::Stop );
                getNode()->setTargetState( 1 );
                lara.setHandStatus( 1 );
            }

            m_triggerState = engine::items::TriggerState::Enabled;

            activate();
            ModelItemNode::update();
        }
    }
}
