#pragma once

#include "statehandler_underwater.h"

namespace engine
{
namespace lara
{
class StateHandler_18 final
        : public StateHandler_Underwater
{
public:
    explicit StateHandler_18(LaraNode& lara)
            : StateHandler_Underwater{lara, LaraStateId::UnderwaterInertia}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health < 0 )
        {
            setGoalAnimState( LaraStateId::WaterDeath );
            return;
        }

        handleDiveRotationInput();

        if( getLevel().m_inputHandler->getInputState().jump )
        {
            setGoalAnimState( LaraStateId::UnderwaterForward );
        }

        getLara().m_state.fallspeed = std::max( 0_len, getLara().m_state.fallspeed - 6_len );
        if( getLara().m_state.fallspeed <= 133_len )
        {
            setGoalAnimState( LaraStateId::UnderwaterStop );
        }
    }
};
}
}
