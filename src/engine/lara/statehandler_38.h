#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine
{
namespace lara
{
class StateHandler_38 final
        : public StateHandler_Pushable
{
public:
    explicit StateHandler_38(LaraNode& lara)
            : StateHandler_Pushable{lara, LaraStateId::PushableGrab}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        setCameraRotationAroundCenterY( 75_deg );
        if( !getEngine().getInputHandler().getInputState().action )
        {
            setGoalAnimState( LaraStateId::Stop );
        }
    }
};
}
}
