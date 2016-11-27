#include "collapsiblefloor.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
    namespace items
    {
        void Item_35_CollapsibleFloor::onFrameChanged(FrameChangeType frameChangeType)
        {
            if( getCurrentState() == 0 ) // stationary
            {
                if( !util::fuzzyEqual( getPosition().Y - 512, getLevel().m_lara->getPosition().Y, 1.0f ) )
                {
                    m_flags2_02_toggledOn = false;
                    m_flags2_04_ready = false;
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

            ItemNode::onFrameChanged( frameChangeType );

            if( m_flags2_04_ready && !m_flags2_02_toggledOn )
            {
                deactivate();
                return;
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findFloorSectorWithClampedPosition( getPosition().toInexact(), &room );
            setCurrentRoom( room );

            HeightInfo h = HeightInfo::fromFloor( sector, getPosition().toInexact(), getLevel().m_cameraController );
            setFloorHeight( h.distance );
            if( getCurrentState() != 2 || getPosition().Y < h.distance )
                return;

            // settle
            setTargetState( 3 );
            setFallSpeed( core::makeInterpolatedValue( 0.0f ) );
            auto pos = getPosition();
            pos.Y = getFloorHeight();
            setPosition( pos );
            setFalling( false );
        }
    }
}
