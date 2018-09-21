#pragma once

#include "statehandler_underwater.h"

namespace engine
{
namespace lara
{
class StateHandler_35 final
        : public StateHandler_Underwater
{
public:
    explicit StateHandler_35(LaraNode& lara)
            : StateHandler_Underwater( lara, LaraStateId::UnderwaterDiving )
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
        {
            getLara().m_state.rotation.X -= 1_deg;
        }
    }
};
}
}
