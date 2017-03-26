#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void TrapDoorUp::update(const std::chrono::microseconds& deltaTime)
        {
            addTime(deltaTime);
            auto pos = getRoomBoundPosition();
            getLevel().findRealFloorSector( pos );
            setCurrentRoom( pos.room );
        }
    }
}
