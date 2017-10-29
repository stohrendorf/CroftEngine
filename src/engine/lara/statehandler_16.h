#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_16 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_16(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::WalkBackward)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.health <= 0 )
        {
            setTargetState(LaraStateId::Stop);
            return;
        }

        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward && getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setTargetState(LaraStateId::WalkBackward);
        }
        else
        {
            setTargetState(LaraStateId::Stop);
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed(2.25_deg, -4_deg);
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed(2.25_deg, 4_deg);
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);

        if( stopIfCeilingBlocked(collisionInfo) )
        {
            return;
        }

        if( checkWallCollision(collisionInfo) )
        {
            setAnimIdGlobal(loader::AnimationId::STAY_SOLID, 185);
        }

        if( collisionInfo.mid.floor.distance > loader::QuarterSectorSize && collisionInfo.mid.floor.distance < core::ClimbLimit2ClickMin )
        {
            if( getLara().m_state.frame_number < 964 || getLara().m_state.frame_number > 993 )
            {
                setAnimIdGlobal(loader::AnimationId::WALK_DOWN_BACK_LEFT, 899);
            }
            else
            {
                setAnimIdGlobal(loader::AnimationId::WALK_DOWN_BACK_RIGHT, 930);
            }
        }

        if( !tryStartSlide(collisionInfo) )
        {
            placeOnFloor(collisionInfo);
        }
    }
};
}
}
