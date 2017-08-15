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
                if( getNode()->getCurrentState() == 0 )
                    getNode()->setTargetState( 2 );
            }
            else if( getNode()->getCurrentState() == 2 )
            {
                getNode()->setTargetState( 0 );
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector(getPosition(), &room);
            setCurrentRoom(room);
            setFloorHeight(HeightInfo::fromFloor(sector, getPosition(), getLevel().m_cameraController)
                           .distance);

            ItemNode::update();
        }
    }
}
