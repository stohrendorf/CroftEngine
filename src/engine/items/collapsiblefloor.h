#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class CollapsibleFloor final : public ItemNode
        {
        public:
            CollapsibleFloor(const gsl::not_null<level::Level*>& level,
                                     const std::string& name,
                                     const gsl::not_null<const loader::Room*>& room,
                                     const gsl::not_null<loader::Item*>& item,
                                     const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x34, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override
            {
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;


            void patchFloor(const core::TRCoordinates& pos, long& y) override
            {
                if( pos.Y > getPosition().Y - 512 )
                    return;

                if( getCurrentState() != 0 && getCurrentState() != 1 )
                    return;

                y = std::lround( getPosition().Y - 512 );
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override
            {
                if( pos.Y <= getPosition().Y - 512 )
                    return;

                if( getCurrentState() != 0 && getCurrentState() != 1 )
                    return;

                y = std::lround( getPosition().Y - 256 );
            }
        };
    }
}
