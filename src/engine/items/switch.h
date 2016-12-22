#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Switch : public ItemNode
        {
        public:
            Switch(const gsl::not_null<level::Level*>& level,
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


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override
            {
                if( !updateTriggerTimeout(deltaTime) )
                {
                    setTargetState(1);
                    m_triggerTimeout = std::chrono::microseconds::zero();
                }
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
