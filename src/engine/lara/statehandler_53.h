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
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 192;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
        checkJumpWallSmash( collisionInfo );
        if( collisionInfo.mid.floor.y > 0 || getLara().m_state.fallspeed <= 0 )
        {
            return;
        }

        if( getLara().m_state.fallspeed <= 133 )
        {
            setGoalAnimState( LaraStateId::Stop );
        }
        else
        {
            setGoalAnimState( LaraStateId::Death );
        }

        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        placeOnFloor( collisionInfo );
    }
};
}
}
