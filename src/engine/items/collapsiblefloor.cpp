#include "collapsiblefloor.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
    namespace items
    {
        void CollapsibleFloor::update()
        {
            if(!m_isActive)
                return;

            if( getCurrentState() == 0 ) // stationary
            {
                if( getPosition().Y - 512 != getLevel().m_lara->getPosition().Y )
                {
                    m_triggerState = TriggerState::Disabled;
                    deactivate();
                    return;
                }
                setTargetState( 1 );
            }
            else if( getCurrentState() == 1 ) // shaking
            {
                setTargetState( 2 );
            }
            else if( getCurrentState() == 2 && getTargetState() != 3 ) // falling, not going to settle
            {
                setFalling( true );
            }

            nextFrame();

            if( m_triggerState == TriggerState::Activated )
            {
                deactivate();
                return;
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector( getPosition(), &room );
            setCurrentRoom( room );

            HeightInfo h = HeightInfo::fromFloor( sector, getPosition(), getLevel().m_cameraController );
            setFloorHeight( h.distance );
            if( getCurrentState() != 2 || getPosition().Y < h.distance )
                return;

            // settle
            setTargetState( 3 );
            setFallSpeed( 0 );
            auto pos = getPosition();
            pos.Y = getFloorHeight();
            setPosition( pos );
            setFalling( false );
        }
    }
}
