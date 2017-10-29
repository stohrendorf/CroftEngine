#pragma once

#include "statehandler_underwater.h"

#include "engine/laranode.h"

namespace engine
{
namespace lara
{
class StateHandler_44 final
    : public StateHandler_Underwater
{
public:
    explicit StateHandler_44(LaraNode& lara)
        : StateHandler_Underwater(lara, LaraStateId::WaterDeath)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        int spd = std::max(0, getLara().m_state.fallspeed - 8);
        getLara().m_state.fallspeed = spd;

        if( getLara().m_state.rotation.X > 2_deg )
        {
            getLara().m_state.rotation.X -= 2_deg;
        }
        else
        {
            if( getLara().m_state.rotation.X < -2_deg )
            {
                getLara().m_state.rotation.X += 2_deg;
            }
            else
            {
                getLara().m_state.rotation.X = 0_deg;
            }
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.health = -1;
        setAir(-1);
        setHandStatus(1);
        auto h = getLara().getWaterSurfaceHeight();
        if( h.is_initialized() && *h < getLara().m_state.position.position.Y - core::DefaultCollisionRadius )
        {
            getLara().m_state.position.position.Y -= 5;
        }

        StateHandler_Underwater::postprocessFrame(collisionInfo);
    }
};
}
}
