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
        : AbstractStateHandler(lara, LaraStateId::Reach)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        setCameraCurrentRotationY(85_deg);
        if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
        {
            setGoalAnimState( LaraStateId::FreeFall );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.falling = true;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = 0;
        collisionInfo.badCeilingDistance = 192;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);

        if( tryReach(collisionInfo) )
        {
            return;
        }

        jumpAgainstWall(collisionInfo);
        if( getLara().m_state.fallspeed <= 0 || collisionInfo.mid.floor.y > 0 )
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

        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        placeOnFloor(collisionInfo);
    }
};
}
}
