#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class PickupItem final : public ItemNode
        {
        public:
            PickupItem(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const core::Angle& angle,
                       const core::ExactTRCoordinates& position,
                       const floordata::ActivationState& activationState,
                       int16_t darkness,
                       const loader::AnimatedModel& animatedModel,
                       bool shotgun = false)
                : ItemNode(level, name, room, angle, position, activationState, true, 0x30, darkness, animatedModel)
                , m_shotgun{shotgun}
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override
            {
            }


            void onInteract(LaraNode& lara) override;


            void onFrameChanged(FrameChangeType frameChangeType) override
            {
            }


        private:
            const bool m_shotgun;
        };
    }
}
