#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine
{
namespace lara
{
class StateHandler_31 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_31(LaraNode& lara)
        : AbstractStateHandler{ lara, LaraStateId::ShimmyRight }
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        setCameraRotationAroundCenter( -60_deg, 0_deg );
        if( getEngine().getInputHandler().getInputState().xMovement != hid::AxisMovement::Right &&
            getEngine().getInputHandler().getInputState().stepMovement != hid::AxisMovement::Right )
        {
            setGoalAnimState( LaraStateId::Hang );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle( getLara().m_state.rotation.Y + 90_deg );
        commonEdgeHangHandling( collisionInfo );
        setMovementAngle( getLara().m_state.rotation.Y + 90_deg );
    }
};
}
}
