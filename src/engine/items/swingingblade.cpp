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
                    setTargetState( 2 );
            }
            else if( getCurrentState() == 2 )
            {
                setTargetState( 0 );
            }

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector(getPosition(), &room);
            setCurrentRoom(room);
            setFloorHeight(HeightInfo::fromFloor(sector, getPosition(), getLevel().m_cameraController)
                           .distance);

            nextFrame();
        }
    }
}
