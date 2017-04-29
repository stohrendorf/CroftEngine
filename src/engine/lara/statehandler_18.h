#pragma once

#include "statehandler_underwater.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_18 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_18(LaraNode& lara)
                : StateHandler_Underwater(lara, LaraStateId::UnderwaterInertia)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() < 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return;
                }

                handleDiveInput();

                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::UnderwaterForward);

                if( getFallSpeed() <= 133 )
                    setTargetState(LaraStateId::UnderwaterStop);

                setFallSpeed(std::max(0, getFallSpeed() - 6));
            }
        };
    }
}
