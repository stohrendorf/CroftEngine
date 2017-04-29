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
                       const core::TRCoordinates& position,
                       const floordata::ActivationState& activationState,
                       int16_t darkness,
                       const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, false, 0, darkness, animatedModel)
            {
            }


            void update() override
            {
                ItemNode::update();
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
