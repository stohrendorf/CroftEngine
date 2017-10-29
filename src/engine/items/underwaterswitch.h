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
                             uint16_t activationState,
                             int16_t darkness,
                             const loader::SkeletalModelType& animatedModel)
                : Switch(level, name, room, angle, position, activationState, darkness, animatedModel)
            {
            }


            void onInteract(LaraNode& lara) override;


            void update() override
            {
                if(!m_isActive)
                    return;

                m_state.activationState.fullyActivate();

                ModelItemNode::update();
            }
        };
    }
}
