#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void TrapDoorUp::update()
        {
            ModelItemNode::update();
            auto pos = getRoomBoundPosition();
            getLevel().findRealFloorSector( pos );
            setCurrentRoom( pos.room );
        }
    }
}
