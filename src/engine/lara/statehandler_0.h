#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "loader/file/level/level.h"
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
        if( getLara().m_state.health <= 0_hp )
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
        getLara().m_state.fallspeed = 0_spd;
        getLara().m_state.falling = false;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle( collisionInfo.facingAngle );
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0_len;
        collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
        collisionInfo.policyFlags.set(CollisionInfo::PolicyFlags::LavaIsPit);
        collisionInfo.initHeightInfo( getLara().m_state.position.position, getLevel(), core::LaraWalkHeight );

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
            if( fr >= 29_frame && fr <= 47_frame )
            {
                setAnimation( AnimationId::END_WALK_LEFT, 74_frame );
            }
            else if( (fr >= 22_frame && fr <= 28_frame) || (fr >= 48_frame && fr <= 57_frame) )
            {
                setAnimation( AnimationId::END_WALK_RIGHT, 58_frame );
            }
            else
            {
                setAnimation( AnimationId::STAY_SOLID, 185_frame );
            }
        }

        if( collisionInfo.mid.floorSpace.y > core::ClimbLimit2ClickMin )
        {
            setAnimation( AnimationId::FREE_FALL_FORWARD, 492_frame );
            setGoalAnimState( LaraStateId::JumpForward );
            getLara().m_state.fallspeed = 0_spd;
            getLara().m_state.falling = true;
        }

        if( collisionInfo.mid.floorSpace.y > core::SteppableHeight )
        {
            const auto fr = getLara().m_state.frame_number;
            if( fr < 28_frame || fr > 45_frame )
            {
                setAnimation( AnimationId::WALK_DOWN_RIGHT, 887_frame );
            }
            else
            {
                setAnimation( AnimationId::WALK_DOWN_LEFT, 874_frame );
            }
        }

        if( collisionInfo.mid.floorSpace.y >= -core::ClimbLimit2ClickMin
            && collisionInfo.mid.floorSpace.y < -core::SteppableHeight )
        {
            const auto fr = getLara().m_state.frame_number;
            if( fr < 27_frame || fr > 44_frame )
            {
                setAnimation( AnimationId::WALK_UP_STEP_RIGHT, 844_frame );
            }
            else
            {
                setAnimation( AnimationId::WALK_UP_STEP_LEFT, 858_frame );
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
