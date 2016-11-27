#pragma once

#include "statehandler_turnslow.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_6 final : public StateHandler_TurnSlow
        {
        public:
            explicit StateHandler_6(LaraNode& lara)
                    : StateHandler_TurnSlow(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return {};
                }

                if( getHandStatus() == 4 )
                {
                    setTargetState(LaraStateId::TurnFast);
                    return {};
                }

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                {
                    if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right )
                        setTargetState(LaraStateId::Stop);
                    return {};
                }

                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState(LaraStateId::WalkForward);
                else
                    setTargetState(LaraStateId::RunForward);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                addYRotationSpeed(deltaTime, 2.25_deg);
                if( getYRotationSpeed() <= 4_deg )
                    return;

                if( !getLevel().m_inputHandler->getInputState().moveSlow )
                    setYRotationSpeed(4_deg);
                else
                    setTargetState(LaraStateId::TurnFast);
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::TurnRightSlow;
            }
        };
    }
}
