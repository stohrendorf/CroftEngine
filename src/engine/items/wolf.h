#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class Wolf final : public ItemNode
        {
        public:
            Wolf(const gsl::not_null<level::Level*>& level,
                          const std::string& name,
                          const gsl::not_null<const loader::Room*>& room,
                          const core::Angle& angle,
                          const core::ExactTRCoordinates& position,
                          const floordata::ActivationState& activationState,
                          int16_t darkness,
                          const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, 0x3e, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;
        };
    }
}