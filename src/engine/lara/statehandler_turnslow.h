#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_TurnSlow
        : public AbstractStateHandler
{
protected:
    explicit StateHandler_TurnSlow(LaraNode& lara, const LaraStateId id)
            : AbstractStateHandler{lara, id}
    {
    }

public:
    void postprocessFrame(CollisionInfo& collisionInfo) override final
    {
        getLara().m_state.fallspeed = 0_len;
        getLara().m_state.falling = false;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );

        if( collisionInfo.mid.floor.y <= core::DefaultCollisionRadius )
        {
            if( !tryStartSlide( collisionInfo ) )
            {
                placeOnFloor( collisionInfo );
            }

            return;
        }

        setAnimation( loader::AnimationId::FREE_FALL_FORWARD, 492 );
        setGoalAnimState( LaraStateId::JumpForward );
        getLara().m_state.fallspeed = 0_len;
        getLara().m_state.falling = true;
    }
};
}
}
