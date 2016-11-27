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
                    : StateHandler_Underwater(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                    m_yRotationSpeed = -1_deg;
                else
                    m_yRotationSpeed = 0_deg;

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::UnderwaterDiving;
            }
        };
    }
}
