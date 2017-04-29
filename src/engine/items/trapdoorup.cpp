#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void TrapDoorUp::update()
        {
            ItemNode::update();
            auto pos = getRoomBoundPosition();
            getLevel().findRealFloorSector( pos );
            setCurrentRoom( pos.room );
        }
    }
}
