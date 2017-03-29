#pragma once

#include "statehandler_turnslow.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_7 final : public StateHandler_TurnSlow
        {
        public:
            explicit StateHandler_7(LaraNode& lara)
                    : StateHandler_TurnSlow(lara, LaraStateId::TurnLeftSlow)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                if( getHandStatus() == 4 )
                {
                    setTargetState(LaraStateId::TurnFast);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                {
                    if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Left )
                        setTargetState(LaraStateId::Stop);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState(LaraStateId::WalkForward);
                else
                    setTargetState(LaraStateId::RunForward);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                subYRotationSpeed(deltaTime, 2.25_deg);
                if( getYRotationSpeed() >= -4_deg )
                    return;

                if( !getLevel().m_inputHandler->getInputState().moveSlow )
                    setYRotationSpeed(-4_deg);
                else
                    setTargetState(LaraStateId::TurnFast);
            }
        };
    }
}
