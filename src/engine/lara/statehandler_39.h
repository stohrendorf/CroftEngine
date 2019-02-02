#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_39 final
        : public AbstractStateHandler
{
public:

    explicit StateHandler_39(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::PickUp}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        setCameraRotationAroundCenter( -15_deg, -130_deg );
        setCameraEyeCenterDistance( 1024 );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
    }
};
}
}
