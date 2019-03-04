#pragma once

#include "statehandler_turnslow.h"

namespace engine
{
namespace lara
{
class StateHandler_6 final
        : public StateHandler_TurnSlow
{
public:
    explicit StateHandler_6(LaraNode& lara)
            : StateHandler_TurnSlow{lara, LaraStateId::TurnRightSlow}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0_hp )
        {
            setGoalAnimState( LaraStateId::Stop );
            return;
        }

        addYRotationSpeed( 2.25_deg );

        if( getHandStatus() == HandStatus::Combat )
        {
            setGoalAnimState( LaraStateId::TurnFast );
        }
        else if( getYRotationSpeed() > 4_deg )
        {
            if( getEngine().m_inputHandler->getInputState().moveSlow )
            {
                setYRotationSpeed( 4_deg );
            }
            else
            {
                setGoalAnimState( LaraStateId::TurnFast );
            }
        }

        if( getEngine().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
        {
            if( getEngine().m_inputHandler->getInputState().xMovement != AxisMovement::Right )
            {
                setGoalAnimState( LaraStateId::Stop );
            }
            return;
        }

        if( getEngine().m_inputHandler->getInputState().moveSlow )
        {
            setGoalAnimState( LaraStateId::WalkForward );
        }
        else
        {
            setGoalAnimState( LaraStateId::RunForward );
        }
    }
};
}
}
