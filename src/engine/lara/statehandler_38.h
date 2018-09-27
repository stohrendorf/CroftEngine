#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

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
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
        setCameraCurrentRotationY( 75_deg );
        if( !getLevel().m_inputHandler->getInputState().action )
        {
            setGoalAnimState( LaraStateId::Stop );
        }
    }
};
}
}
