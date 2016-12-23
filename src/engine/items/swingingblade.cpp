#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "level/level.h"

namespace engine
{
    namespace items
    {
        void SwingingBlade::updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/)
        {
            if( updateActivationTimeout( deltaTime ) )
            {
                if( getCurrentState() == 0 )
                    setTargetState( 2 );
            }
            else if( getCurrentState() == 2 )
            {
                setTargetState( 0 );
            }
        }


        void SwingingBlade::onFrameChanged(FrameChangeType frameChangeType)
        {
            auto room = getCurrentRoom();
            auto sector = getLevel().findFloorSectorWithClampedPosition( getPosition().toInexact(), &room );
            setCurrentRoom( room );
            setFloorHeight( HeightInfo::fromFloor( sector, getPosition().toInexact(), getLevel().m_cameraController )
                                    .distance );

            ItemNode::onFrameChanged( frameChangeType );
        }
    }
}
