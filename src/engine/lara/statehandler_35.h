#pragma once

#include "statehandler_underwater.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_35 final : public StateHandler_Underwater
        {
        public:
            explicit StateHandler_35(LaraNode& lara)
                    : StateHandler_Underwater(lara, LaraStateId::UnderwaterDiving)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                    m_yRotationSpeed = -1_deg;
                else
                    m_yRotationSpeed = 0_deg;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }
        };
    }
}
