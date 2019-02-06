#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_11 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_11(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::Reach}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        setCameraRotationAroundCenterY( 85_deg );
        if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
        {
            setGoalAnimState( LaraStateId::FreeFall );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.falling = true;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::HeightLimit;
        collisionInfo.badNegativeDistance = 0_len;
        collisionInfo.badCeilingDistance = 192_len;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::LaraWalkHeight );

        if( tryReach( collisionInfo ) )
        {
            return;
        }

        jumpAgainstWall( collisionInfo );
        if( getLara().m_state.fallspeed <= 0_len || collisionInfo.mid.floorSpace.y > 0_len )
        {
            return;
        }

        if( applyLandingDamage() )
        {
            setGoalAnimState( LaraStateId::Death );
        }
        else
        {
            setGoalAnimState( LaraStateId::Stop );
        }

        getLara().m_state.fallspeed = 0_len;
        getLara().m_state.falling = false;
        placeOnFloor( collisionInfo );
    }
};
}
}
