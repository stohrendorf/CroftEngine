#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void TrapDoorUp::onFrameChanged(FrameChangeType frameChangeType)
        {
            ItemNode::onFrameChanged( frameChangeType );
            auto pos = getRoomBoundPosition();
            getLevel().findRealFloorSector( pos );
            setCurrentRoom( pos.room );
        }
    }
}
