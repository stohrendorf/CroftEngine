#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
namespace lara
{
class StateHandler_9 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_9(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::FreeFall)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        if( getLara().m_state.fallspeed >= core::DeadlyFallSpeedThreshold )
        {
            getLara().playSoundEffect(30);
        }
        dampenHorizontalSpeed(0.05f);
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 192;
        collisionInfo.facingAngle = getMovementAngle();
        getLara().m_state.falling = true;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);
        jumpAgainstWall(collisionInfo);
        if( collisionInfo.mid.floor.distance > 0 )
        {
            return;
        }

        if( applyLandingDamage() )
        {
            setTargetState(LaraStateId::Death);
        }
        else
        {
            setTargetState(LaraStateId::Stop);
            setAnimIdGlobal(loader::AnimationId::LANDING_HARD, 358);
        }
        getLevel().stopSoundEffect(30);
        getLara().m_state.fallspeed = 0;
        placeOnFloor(collisionInfo);
        getLara().m_state.falling = false;
    }
};
}
}
