#pragma once

#include "switch.h"


namespace engine
{
    namespace items
    {
        class UnderwaterSwitch final : public Switch
        {
        public:
            UnderwaterSwitch(const gsl::not_null<level::Level*>& level,
                             const std::string& name,
                             const gsl::not_null<const loader::Room*>& room,
                             const core::Angle& angle,
                             const core::TRCoordinates& position,
                             const floordata::ActivationState& activationState,
                             int16_t darkness,
                             const loader::AnimatedModel& animatedModel)
                : Switch(level, name, room, angle, position, activationState, darkness, animatedModel)
            {
            }


            void onInteract(LaraNode& lara) override;


            void update() override
            {
                if(!m_isActive)
                    return;

                m_activationState.fullyActivate();

                nextFrame();
            }
        };
    }
}
