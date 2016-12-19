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
                             const core::ExactTRCoordinates& position,
                             uint16_t flags,
                             int16_t darkness,
                             const loader::AnimatedModel& animatedModel)
                : Switch(level, name, room, angle, position, flags, darkness, animatedModel)
            {
            }


            void onInteract(LaraNode& lara) override;


            void onFrameChanged(FrameChangeType frameChangeType) override
            {
                m_itemFlags |= ActivationMask;

                ItemNode::onFrameChanged(frameChangeType);
            }
        };
    }
}
