#pragma once

#include "statehandler_underwater.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_13 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_13(LaraNode& lara)
                : StateHandler_Underwater(lara, LaraStateId::UnderwaterStop)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() < 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                }

                handleDiveInput();

                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::UnderwaterForward);

                setFallSpeed(std::max(0, getFallSpeed() - 6));
            }
        };
    }
}
