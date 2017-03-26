#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class StubItem final : public ItemNode
        {
        public:
            StubItem(const gsl::not_null<level::Level*>& level,
                     const std::string& name,
                     const gsl::not_null<const loader::Room*>& room,
                     const core::Angle& angle,
                     const core::ExactTRCoordinates& position,
                     const floordata::ActivationState& activationState,
                     int16_t darkness,
                     const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, false, 0, darkness, animatedModel)
            {
            }


            void update(const std::chrono::microseconds& deltaTime) override
            {
                addTime(deltaTime);
            }
        };
    }
}
