#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace lara
{
class StateHandler_0 final
        : public AbstractStateHandler
{
public:

    explicit StateHandler_0(LaraNode& lara)
            : AbstractStateHandler{lara, LaraStateId::WalkForward}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0 )
        {
            setGoalAnimState( LaraStateId::Stop );
            return;
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed( 2.25_deg, -4_deg );
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed( 2.25_deg, 4_deg );
        }

        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
        {
            if( getLevel().m_inputHandler->getInputState().moveSlow )
            {
                setGoalAnimState( LaraStateId::WalkForward );
            }
            else
            {
                setGoalAnimState( LaraStateId::RunForward );
            }
        }
        else
        {
            setGoalAnimState( LaraStateId::Stop );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits
                                     | CollisionInfo::LavaIsPit;
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::ScalpHeight );

        if( stopIfCeilingBlocked( collisionInfo ) )
        {
            return;
        }

        if( tryClimb( collisionInfo ) )
        {
            return;
        }

        if( checkWallCollision( collisionInfo ) )
        {
            const auto fr = getLara().m_state.frame_number;
            if( fr >= 29 && fr <= 47 )
            {
                setAnimation( loader::AnimationId::END_WALK_LEFT, 74 );
            }
            else if( (fr >= 22 && fr <= 28) || (fr >= 48 && fr <= 57) )
            {
                setAnimation( loader::AnimationId::END_WALK_RIGHT, 58 );
            }
            else
            {
                setAnimation( loader::AnimationId::STAY_SOLID, 185 );
            }
        }

        if( collisionInfo.mid.floor.y > core::ClimbLimit2ClickMin )
        {
            setAnimation( loader::AnimationId::FREE_FALL_FORWARD, 492 );
            setGoalAnimState( LaraStateId::JumpForward );
            getLara().m_state.fallspeed = 0;
            getLara().m_state.falling = true;
        }

        if( collisionInfo.mid.floor.y > core::SteppableHeight )
        {
            const auto fr = getLara().m_state.frame_number;
            if( fr < 28 || fr > 45 )
            {
                setAnimation( loader::AnimationId::WALK_DOWN_RIGHT, 887 );
            }
            else
            {
                setAnimation( loader::AnimationId::WALK_DOWN_LEFT, 874 );
            }
        }

        if( collisionInfo.mid.floor.y >= -core::ClimbLimit2ClickMin
            && collisionInfo.mid.floor.y < -core::SteppableHeight )
        {
            const auto fr = getLara().m_state.frame_number;
            if( fr < 27 || fr > 44 )
            {
                setAnimation( loader::AnimationId::WALK_UP_STEP_RIGHT, 844 );
            }
            else
            {
                setAnimation( loader::AnimationId::WALK_UP_STEP_LEFT, 858 );
            }
        }

        if( !tryStartSlide( collisionInfo ) )
        {
            placeOnFloor( collisionInfo );
        }
    }
};
}
}
