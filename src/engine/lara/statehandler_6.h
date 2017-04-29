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
                : StateHandler_TurnSlow(lara, LaraStateId::TurnRightSlow)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                addYRotationSpeed(2.25_deg);

                if( getHandStatus() == 4 )
                {
                    setTargetState(LaraStateId::TurnFast);
                }
                else if( getYRotationSpeed() > 4_deg )
                {
                    if( !getLevel().m_inputHandler->getInputState().moveSlow )
                        setYRotationSpeed(4_deg);
                    else
                        setTargetState(LaraStateId::TurnFast);
                }

                if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
                {
                    if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right )
                        setTargetState(LaraStateId::Stop);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().moveSlow )
                    setTargetState(LaraStateId::WalkForward);
                else
                    setTargetState(LaraStateId::RunForward);
            }
        };
    }
}
