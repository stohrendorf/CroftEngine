#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_15 final
        : public AbstractStateHandler
{
public:
    explicit StateHandler_15(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::JumpPrepare}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward &&
            getRelativeHeightAtDirection( getLara().m_state.rotation.Y, 256 ) >= -core::ClimbLimit2ClickMin )
        {
            setMovementAngle( getLara().m_state.rotation.Y );
            setGoalAnimState( LaraStateId::JumpForward );
        }
        else
        {
            if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left
                && getRelativeHeightAtDirection(
                        getLara().m_state.rotation.Y - 90_deg, 256 ) >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle( getLara().m_state.rotation.Y - 90_deg );
                setGoalAnimState( LaraStateId::JumpRight );
            }
            else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right &&
                     getRelativeHeightAtDirection( getLara().m_state.rotation.Y + 90_deg, 256 )
                     >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle( getLara().m_state.rotation.Y + 90_deg );
                setGoalAnimState( LaraStateId::JumpLeft );
            }
            else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward &&
                     getRelativeHeightAtDirection( getLara().m_state.rotation.Y + 180_deg, 256 )
                     >= -core::ClimbLimit2ClickMin )
            {
                setMovementAngle( getLara().m_state.rotation.Y + 180_deg );
                setGoalAnimState( LaraStateId::JumpBack );
            }
        }

        if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
        {
            setGoalAnimState( LaraStateId::FreeFall );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -loader::HeightLimit;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.facingAngle = getMovementAngle();
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );

        if( collisionInfo.mid.ceiling.y <= -core::DefaultCollisionRadius )
        {
            return;
        }

        setAnimation( loader::AnimationId::STAY_SOLID, 185 );
        setGoalAnimState( LaraStateId::Stop );
        getLara().m_state.speed = 0;
        getLara().m_state.position.position = collisionInfo.oldPosition;
    }
};
}
}
