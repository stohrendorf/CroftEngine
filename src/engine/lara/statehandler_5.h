#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_5 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_5(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::RunBack}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        setGoalAnimState( LaraStateId::Stop );

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed( 2.25_deg, -6_deg );
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed( 2.25_deg, 6_deg );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.fallspeed = 0_len;
        getLara().m_state.falling = false;
        collisionInfo.badPositiveDistance = core::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );
        if( stopIfCeilingBlocked( collisionInfo ) )
        {
            return;
        }

        if( collisionInfo.mid.floor.y > 200_len )
        {
            setAnimation( loader::AnimationId::FREE_FALL_BACK, 1473 );
            setGoalAnimState( LaraStateId::FallBackward );
            getLara().m_state.fallspeed = 0_len;
            getLara().m_state.falling = true;
            return;
        }

        if( checkWallCollision( collisionInfo ) )
        {
            setAnimation( loader::AnimationId::STAY_SOLID, 185 );
        }
        placeOnFloor( collisionInfo );
    }
};
}
}
