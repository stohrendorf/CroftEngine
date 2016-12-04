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

            if( m_flags2_04_ready || m_flags2_02_toggledOn )
                return;

            if( lara.getCurrentState() != loader::LaraStateId::Stop )
                return;

            static const InteractionLimits limits{
                    gameplay::BoundingBox{core::TRCoordinates{-200, 0, 312}.toRenderSystem(),
                                          core::TRCoordinates{+200, 0, 512}.toRenderSystem()},
                    {-10_deg, -30_deg, -10_deg},
                    {+10_deg, +30_deg, +10_deg}
            };

            if( !limits.canInteract( *this, lara ) )
                return;

            lara.setYRotation( getRotation().Y );

            if( getCurrentState() == 1 )
            {
                BOOST_LOG_TRIVIAL( debug ) << "Switch " << getId() << ": pull down";
                do
                {
                    lara.setTargetState( loader::LaraStateId::SwitchDown );
                    lara.advanceFrame();
                } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown );
                lara.setTargetState( loader::LaraStateId::Stop );
                setTargetState( 0 );
                lara.setHandStatus( 1 );
            }
            else
            {
                if( getCurrentState() != 0 )
                    return;

                BOOST_LOG_TRIVIAL( debug ) << "Switch " << getId() << ": pull up";
                do
                {
                    lara.setTargetState( loader::LaraStateId::SwitchUp );
                    lara.advanceFrame();
                } while( lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp );
                lara.setTargetState( loader::LaraStateId::Stop );
                setTargetState( 1 );
                lara.setHandStatus( 1 );
            }

            m_flags2_04_ready = false;
            m_flags2_02_toggledOn = true;

            activate();
        }
    }
}
