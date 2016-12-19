#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class SwingingBlade final : public ItemNode
        {
        public:
            SwingingBlade(const gsl::not_null<level::Level*>& level,
                          const std::string& name,
                          const gsl::not_null<const loader::Room*>& room,
                          const core::Angle& angle,
                          const core::ExactTRCoordinates& position,
                          uint16_t flags,
                          int16_t darkness,
                          const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, flags, true, 0x30, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;
        };
    }
}
