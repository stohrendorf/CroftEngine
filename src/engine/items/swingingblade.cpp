#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "level/level.h"

namespace engine
{
    namespace items
    {
        void SwingingBlade::update(const std::chrono::microseconds& deltaTime)
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

            auto room = getCurrentRoom();
            auto sector = getLevel().findRealFloorSector(getPosition().toInexact(), &room);
            setCurrentRoom(room);
            setFloorHeight(HeightInfo::fromFloor(sector, getPosition().toInexact(), getLevel().m_cameraController)
                           .distance);

            addTime(deltaTime);
        }
    }
}
