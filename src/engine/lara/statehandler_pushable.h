#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_Pushable
    : public AbstractStateHandler
{
public:
    explicit StateHandler_Pushable(LaraNode& lara, const LaraStateId id)
        : AbstractStateHandler{ lara, id }
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
        setCameraModifier( CameraModifier::FollowCenter );
        setCameraRotationAroundCenter( -25_deg, 35_deg );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) final
    {
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;

        collisionInfo.initHeightInfo( getLara().m_state.position.position, getEngine(), core::LaraWalkHeight );
    }
};
}
}
