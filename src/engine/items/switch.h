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
                   const core::TRCoordinates& position,
                   const floordata::ActivationState& activationState,
                   int16_t darkness,
                   const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags, darkness, animatedModel)
            {
            }


            void onInteract(LaraNode& lara) override;


            void update() override
            {
                if(!m_isActive)
                    return;

                if(!updateActivationTimeout())
                {
                    setTargetState(1);
                    m_activationState.setTimeout(0);
                }

                m_activationState.fullyActivate();

                nextFrame();
            }
        };
    }
}
