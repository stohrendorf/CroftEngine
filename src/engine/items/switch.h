#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Switch : public ModelItemNode
        {
        public:
            Switch(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const core::Angle& angle,
                   const core::TRCoordinates& position,
                   uint16_t activationState,
                   int16_t darkness,
                   const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags, darkness, animatedModel)
            {
            }


            void onInteract(LaraNode& lara) override;


            void update() override
            {
                if(!m_state.updateActivationTimeout())
                {
                    m_state.goal_anim_state = 1;
                    m_state.timer = 0;
                }

                m_state.activationState.fullyActivate();

                ModelItemNode::update();
            }
        };
    }
}
