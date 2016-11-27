#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class Item_68_BridgeFlat final : public ItemNode
        {
        public:
            Item_68_BridgeFlat(const gsl::not_null<level::Level*>& level,
                               const std::string& name,
                               const gsl::not_null<const loader::Room*>& room,
                               const gsl::not_null<loader::Item*>& item,
                               const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, false, 0, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override
            {
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType /*frameChangeType*/) override
            {
            }


            void patchFloor(const core::TRCoordinates& pos, long& y) override
            {
                if( pos.Y <= getPosition().Y )
                    y = std::lround( getPosition().Y );
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override
            {
                if( pos.Y <= getPosition().Y )
                    return;

                y = std::lround( getPosition().Y + loader::QuarterSectorSize );
            }
        };
    }
}
