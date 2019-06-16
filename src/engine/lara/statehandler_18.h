#pragma once

#include "statehandler_underwater.h"

namespace engine
{
namespace lara
{
class StateHandler_18 final : public StateHandler_Underwater
{
public:
    explicit StateHandler_18(LaraNode& lara)
        : StateHandler_Underwater{lara, LaraStateId::UnderwaterInertia}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if(getLara().m_state.health < 0_hp)
        {
            setGoalAnimState(LaraStateId::WaterDeath);
            return;
        }

        handleDiveRotationInput();

        if(getEngine().getInputHandler().getInputState().jump)
        {
            setGoalAnimState(LaraStateId::UnderwaterForward);
        }

        getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - 6_spd);
        if(getLara().m_state.fallspeed <= core::DeadlyHeadFallSpeedThreshold)
        {
            setGoalAnimState(LaraStateId::UnderwaterStop);
        }
    }
};
} // namespace lara
} // namespace engine
