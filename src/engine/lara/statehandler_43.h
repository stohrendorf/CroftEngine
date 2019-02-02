#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_43 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_43(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::InsertPuzzle}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        setCameraRotationAroundCenter( -25_deg, -80_deg );
        setCameraEyeCenterDistance( 1024 );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle( getLara().m_state.rotation.Y );
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
    }
};
}
}
