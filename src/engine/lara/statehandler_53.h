#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_53 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_53(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::SwandiveEnd}
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags.reset( CollisionInfo::PolicyFlags::EnableSpaz );
        collisionInfo.policyFlags.set( CollisionInfo::PolicyFlags::EnableBaddiePush );

        dampenHorizontalSpeed( 0.05f );
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = core::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 192_len;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::LaraWalkHeight );
        checkJumpWallSmash( collisionInfo );
        if( collisionInfo.mid.floorSpace.y > 0_len || getLara().m_state.fallspeed <= 0_len )
        {
            return;
        }

        if( getLara().m_state.fallspeed <= 133_len )
        {
            setGoalAnimState( LaraStateId::Stop );
        }
        else
        {
            setGoalAnimState( LaraStateId::Death );
        }

        getLara().m_state.fallspeed = 0_len;
        getLara().m_state.falling = false;
        placeOnFloor( collisionInfo );
    }
};
}
}
