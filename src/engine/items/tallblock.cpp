#include "tallblock.h"

#include "level/level.h"

namespace engine
{
    namespace items
    {
        void TallBlock::update()
        {
            if(updateActivationTimeout())
            {
                if(getNode()->getCurrentState() == 0)
                {
                    loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                    getNode()->setTargetState(1);
                }
            }
            else
            {
                if(getNode()->getCurrentState() == 1)
                {
                    loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                    getNode()->setTargetState(0);
                }
            }

            ModelItemNode::update();
            auto room = getCurrentRoom();
            getLevel().findRealFloorSector( getPosition(), &room );
            setCurrentRoom( room );

            if( m_triggerState != engine::items::TriggerState::Activated )
                return;

            m_triggerState = engine::items::TriggerState::Enabled;
            loader::Room::patchHeightsForBlock( *this, -2 * loader::SectorSize );
            auto pos = getPosition();
            pos.X = ( pos.X / loader::SectorSize ) * loader::SectorSize + loader::SectorSize / 2;
            pos.Z = ( pos.Z / loader::SectorSize ) * loader::SectorSize + loader::SectorSize / 2;
            setPosition( pos );
        }
    }
}
