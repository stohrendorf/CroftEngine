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
class StateHandler_1 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_1(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::RunForward)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0 )
        {
            setGoalAnimState( LaraStateId::Death );
            return;
        }

        if( getLevel().m_inputHandler->getInputState().roll )
        {
            setAnimIdGlobal(loader::AnimationId::ROLL_BEGIN, 3857);
            setGoalAnimState( LaraStateId::Stop );
            return;
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed(2.25_deg, -8_deg);
            core::Angle z = std::max(-11_deg, getLara().m_state.rotation.Z - 1.5_deg);
            getLara().m_state.rotation.Z = z;
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed(2.25_deg, 8_deg);
            core::Angle z = std::min(+11_deg, getLara().m_state.rotation.Z + 1.5_deg);
            getLara().m_state.rotation.Z = z;
        }

        if( getLevel().m_inputHandler->getInputState().jump && !getLara().m_state.falling )
        {
            setGoalAnimState( LaraStateId::JumpForward );
            return;
        }

        if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward )
        {
            setGoalAnimState( LaraStateId::Stop );
            return;
        }

        if( getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setGoalAnimState( LaraStateId::WalkForward );
        }
        else
        {
            setGoalAnimState( LaraStateId::RunForward );
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);

        if( stopIfCeilingBlocked(collisionInfo) )
        {
            return;
        }

        if( tryClimb(collisionInfo) )
        {
            return;
        }

        if( checkWallCollision(collisionInfo) )
        {
            getLara().m_state.rotation.Z = 0_deg;
            if( collisionInfo.front.floor.slantClass == SlantClass::None
                && collisionInfo.front.floor.y < -core::ClimbLimit2ClickMax )
            {
                if( getLara().m_state.frame_number < 10 )
                {
                    setAnimIdGlobal(loader::AnimationId::WALL_SMASH_LEFT, 800);
                    return;
                }
                if( getLara().m_state.frame_number >= 10 && getLara().m_state.frame_number < 22 )
                {
                    setAnimIdGlobal(loader::AnimationId::WALL_SMASH_RIGHT, 815);
                    return;
                }
            }

            setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
        }

        if( collisionInfo.mid.floor.y > core::ClimbLimit2ClickMin )
        {
            setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
            setGoalAnimState( LaraStateId::JumpForward );
            getLara().m_state.falling = true;
            getLara().m_state.fallspeed = 0;
            return;
        }

        if( collisionInfo.mid.floor.y >= -core::ClimbLimit2ClickMin
            && collisionInfo.mid.floor.y < -core::SteppableHeight )
        {
            if( getLara().m_state.frame_number >= 3 && getLara().m_state.frame_number <= 14 )
            {
                setAnimIdGlobal(loader::AnimationId::RUN_UP_STEP_LEFT, 837);
            }
            else
            {
                setAnimIdGlobal(loader::AnimationId::RUN_UP_STEP_RIGHT, 830);
            }
        }

        if( !tryStartSlide(collisionInfo) )
        {
            int d = std::min(collisionInfo.mid.floor.y, 50);
            getLara().m_state.position.position.Y += d;
        }
    }
};
}
}
