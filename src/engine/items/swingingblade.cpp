#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "level/level.h"

namespace engine
{
    namespace items
    {
        void SwingingBlade::update()
        {
            if( updateActivationTimeout() )
            {
                if( getCurrentState() == 0 )
                    getSkeleton()->setTargetState( 2 );
            }
            else if( getCurrentState() == 2 )
            {
                getSkeleton()->setTargetState( 0 );
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector(getPosition(), &room);
            setCurrentRoom(room);
            setFloorHeight(HeightInfo::fromFloor(sector, getPosition(), getLevel().m_cameraController)
                           .distance);

            ModelItemNode::update();
        }
    }
}
