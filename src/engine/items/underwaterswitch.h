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
                             const loader::Item& item,
                             const loader::SkeletalModelType& animatedModel)
                : Switch(level, name, room, item, animatedModel)
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
