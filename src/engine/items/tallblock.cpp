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
                if(getCurrentState() == 0)
                {
                    loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                    setTargetState(1);
                }
            }
            else
            {
                if(getCurrentState() == 1)
                {
                    loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                    setTargetState(0);
                }
            }

            nextFrame();
            auto room = getCurrentRoom();
            getLevel().findRealFloorSector( getPosition(), &room );
            setCurrentRoom( room );

            if( m_triggerState != engine::items::TriggerState::Activated )
                return;

            m_triggerState = engine::items::TriggerState::Enabled;
            loader::Room::patchHeightsForBlock( *this, -2 * loader::SectorSize );
            auto pos = getPosition();
            pos.X = std::floor( pos.X / loader::SectorSize ) * loader::SectorSize + loader::SectorSize / 2;
            pos.Z = std::floor( pos.Z / loader::SectorSize ) * loader::SectorSize + loader::SectorSize / 2;
            setPosition( pos );
        }
    }
}
