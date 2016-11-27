#pragma once

#include "statehandler_standing.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_20 final : public StateHandler_Standing
        {
        public:
            explicit StateHandler_20(LaraNode& lara)
                    : StateHandler_Standing(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return nullptr;
                }

                if( getYRotationSpeed() >= 0_deg )
                {
                    setYRotationSpeed(8_deg);
                    if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                        return nullptr;
                }
                else
                {
                    setYRotationSpeed(-8_deg);
                    if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                        return nullptr;
                }

                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::TurnFast;
            }
        };
    }
}
