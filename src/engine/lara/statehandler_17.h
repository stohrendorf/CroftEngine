#pragma once

#include "statehandler_underwater.h"

namespace engine
{
namespace lara
{
class StateHandler_17 final
        : public StateHandler_Underwater
{
public:
    explicit StateHandler_17(LaraNode& lara)
            : StateHandler_Underwater{lara, LaraStateId::UnderwaterForward}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health < 0_hp )
        {
            setGoalAnimState( LaraStateId::WaterDeath );
            return;
        }

        handleDiveRotationInput();

        if( !getLevel().m_inputHandler->getInputState().jump )
        {
            setGoalAnimState( LaraStateId::UnderwaterInertia );
        }

        getLara().m_state.fallspeed = std::min( getLara().m_state.fallspeed + 8_spd, 200_spd );
    }
};
}
}
