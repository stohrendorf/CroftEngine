#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_8 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_8(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::Death}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        collisionInfo.collisionRadius = 400_len;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        applyShift( collisionInfo );
        placeOnFloor( collisionInfo );
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::LaraWalkHeight );
        getLara().m_state.health = -1_hp;
        setAir( -1 );
    }
};
}
}
