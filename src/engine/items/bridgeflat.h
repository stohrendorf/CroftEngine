#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class BridgeFlat final : public ItemNode
        {
        public:
            BridgeFlat(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const core::Angle& angle,
                       const core::ExactTRCoordinates& position,
                       uint16_t flags,
                       int16_t darkness,
                       const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, false, 0, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/, const boost::optional<FrameChangeType>& /*frameChangeType*/) override
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
                    y = std::lround(getPosition().Y);
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override
            {
                if( pos.Y <= getPosition().Y )
                    return;

                y = std::lround(getPosition().Y + loader::QuarterSectorSize);
            }
        };
    }
}
