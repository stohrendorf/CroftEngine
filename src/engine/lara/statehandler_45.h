#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_45 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_45(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::RollForward)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        getLara().m_state.falling = false;
        getLara().m_state.fallspeed = 0;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls;
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);

        if( stopIfCeilingBlocked(collisionInfo) )
        {
            return;
        }

        if( tryStartSlide(collisionInfo) )
        {
            return;
        }

        if( collisionInfo.mid.floor.y <= 200 )
        {
            applyShift(collisionInfo);
            placeOnFloor(collisionInfo);
            return;
        }

        setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setTargetState(LaraStateId::JumpForward);
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = true;
    }
};
}
}
