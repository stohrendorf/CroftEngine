#pragma once

#include "statehandler_turnslow.h"

namespace engine
{
namespace lara
{
class StateHandler_7 final
    : public StateHandler_TurnSlow
{
public:
    explicit StateHandler_7(LaraNode& lara)
        : StateHandler_TurnSlow(lara, LaraStateId::TurnLeftSlow)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return;
        }

        subYRotationSpeed(2.25_deg);

        if( getHandStatus() == 4 )
        {
            setTargetState(LaraStateId::TurnFast);
        }
        else if( getYRotationSpeed() < -4_deg )
        {
            if( getLevel().m_inputHandler->getInputState().moveSlow )
            {
                setYRotationSpeed(-4_deg);
            }
            else
            {
                setTargetState(LaraStateId::TurnFast);
            }
        }

        if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
        {
            if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Left )
            {
                setTargetState(LaraStateId::Stop);
            }
            return;
        }

        if( getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setTargetState(LaraStateId::WalkForward);
        }
        else
        {
            setTargetState(LaraStateId::RunForward);
        }
    }
};
}
}
