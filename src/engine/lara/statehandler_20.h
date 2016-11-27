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

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return {};
                }

                if( getYRotationSpeed() >= 0_deg )
                {
                    setYRotationSpeed(8_deg);
                    if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                        return {};
                }
                else
                {
                    setYRotationSpeed(-8_deg);
                    if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                        return {};
                }

                setTargetState(LaraStateId::Stop);
                return {};
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
