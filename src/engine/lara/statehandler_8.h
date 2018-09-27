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
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.collisionRadius = 400;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        applyShift( collisionInfo );
        placeOnFloor( collisionInfo );
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
        getLara().m_state.health = -1;
        setAir( -1 );
    }
};
}
}
