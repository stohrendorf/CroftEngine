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
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, item, true, SaveHitpoints | SaveFlags, animatedModel)
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
