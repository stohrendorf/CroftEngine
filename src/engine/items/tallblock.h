#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class TallBlock final : public ItemNode
        {
        public:
            TallBlock(const gsl::not_null<level::Level*>& level,
                           const std::string& name,
                           const gsl::not_null<const loader::Room*>& room,
                           const gsl::not_null<loader::Item*>& item,
                           const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x34, animatedModel )
            {
                loader::Room::patchHeightsForBlock( *this, -2 * loader::SectorSize );
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( updateTriggerTimeout( deltaTime ) )
                {
                    if( getCurrentState() == 0 )
                    {
                        loader::Room::patchHeightsForBlock( *this, 2 * loader::SectorSize );
                        setTargetState( 1 );
                    }
                }
                else
                {
                    if( getCurrentState() == 1 )
                    {
                        loader::Room::patchHeightsForBlock( *this, 2 * loader::SectorSize );
                        setTargetState( 0 );
                    }
                }
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;
        };
    }
}
