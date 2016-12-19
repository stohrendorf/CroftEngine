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
                     uint16_t flags,
                     int16_t darkness,
                     const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, false, 0, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override
            {
            }
        };
    }
}
