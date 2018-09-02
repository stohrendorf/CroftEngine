#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_3 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_3(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::JumpForward)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
        {
            setTargetState(LaraStateId::JumpForward);
        }

        if( getTargetState() != LaraStateId::Death && getTargetState() != LaraStateId::Stop )
        {
            if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == HandStatus::None )
            {
                setTargetState(LaraStateId::Reach);
            }

            if( getLevel().m_inputHandler->getInputState().moveSlow && getHandStatus() == HandStatus::None )
            {
                setTargetState(LaraStateId::SwandiveBegin);
            }

            if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
            {
                setTargetState(LaraStateId::FreeFall);
            }
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            subYRotationSpeed(2.25_deg, -3_deg);
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            addYRotationSpeed(2.25_deg, 3_deg);
        }
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 192;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);
        checkJumpWallSmash(collisionInfo);

        if( collisionInfo.mid.floor.y > 0 || getLara().m_state.fallspeed <= 0 )
        {
            return;
        }

        if( applyLandingDamage() )
        {
            setTargetState(LaraStateId::Death);
        }
        else if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward || getLevel().m_inputHandler->getInputState().moveSlow )
        {
            setTargetState(LaraStateId::Stop);
        }
        else
        {
            setTargetState(LaraStateId::RunForward);
        }

        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        getLara().m_state.speed = 0;
        placeOnFloor(collisionInfo);

        laraUpdateImpl();
    }
};
}
}
