#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_41 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_41(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::SwitchUp}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush);
        setCameraCurrentRotation( -25_deg, 80_deg );
        setCameraTargetDistance( 1024 );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle( getLara().m_state.rotation.Y );
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;

        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
    }
};
}
}
