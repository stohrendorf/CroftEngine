#pragma once

#include "statehandler_onwater.h"

namespace engine
{
namespace lara
{
class StateHandler_49 final
    : public StateHandler_OnWater
{
public:
    explicit StateHandler_49(LaraNode& lara)
        : StateHandler_OnWater(lara, LaraStateId::OnWaterRight)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0 )
        {
            setGoalAnimState( LaraStateId::WaterDeath );
            return;
        }

        setSwimToDiveKeypressDuration(0);

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            getLara().m_state.rotation.Y -= 2_deg;
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            getLara().m_state.rotation.Y += 2_deg;
        }

        if( getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
        {
            setGoalAnimState( LaraStateId::OnWaterStop );
        }

        int spd = std::min(60, getLara().m_state.fallspeed + 8);
        getLara().m_state.fallspeed = spd;
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
        commonOnWaterHandling(collisionInfo);
    }
};
}
}
