#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void Item_41_TrapDoorUp::onFrameChanged(FrameChangeType frameChangeType)
        {
            ItemNode::onFrameChanged( frameChangeType );
            auto pos = getRoomBoundPosition();
            getLevel().findFloorSectorWithClampedPosition( pos );
            setCurrentRoom( pos.room );
        }
    }
}
