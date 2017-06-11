#include "underwaterswitch.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
    namespace items
    {
        void UnderwaterSwitch::onInteract(LaraNode& lara)
        {
            if( !getLevel().m_inputHandler->getInputState().action )
                return;

            if(m_triggerState != engine::items::TriggerState::Disabled)
                return;

            if(!lara.isDiving())
                return;

            if(lara.getCurrentAnimState() != LaraStateId::UnderwaterStop)
                return;

            static const InteractionLimits limits{
                core::BoundingBox{ { -1024, -1024, -1024 }, { 1024, 1024, 512 } },
                { -80_deg, -80_deg, -80_deg },
                { +80_deg, +80_deg, +80_deg }
            };

            if(!limits.canInteract(*this, lara))
                return;

            if(getCurrentState() != 0 && getCurrentState() != 1)
                return;

            static const glm::vec3 alignSpeed{ 0, 0, -108.0f };
            if(!lara.alignTransform(alignSpeed, *this))
                return;

            lara.setFallSpeed(0);
            do
            {
                lara.setTargetState(LaraStateId::SwitchDown);
                lara.updateImpl();
            } while(lara.getCurrentAnimState() != LaraStateId::SwitchDown);
            lara.setTargetState(LaraStateId::UnderwaterStop);
            lara.setHandStatus(1);
            m_triggerState = engine::items::TriggerState::Enabled;

            if(getCurrentState() == 1)
                setTargetState(0);
            else
                setTargetState(1);

            activate();
        }
    }
}
