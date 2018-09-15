#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_Standing
    : public AbstractStateHandler
{
protected:
    explicit StateHandler_Standing(LaraNode& lara, LaraStateId id)
        : AbstractStateHandler(lara, id)
    {
    }

public:
    void postprocessFrame(CollisionInfo& collisionInfo) override final
    {
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        collisionInfo.facingAngle = getLara().m_state.rotation.Y;
        setMovementAngle(collisionInfo.facingAngle);
        collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
        collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
        collisionInfo.badCeilingDistance = 0;
        collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
        collisionInfo.initHeightInfo(getLara().m_state.position.position, getLevel(), core::ScalpHeight);

        if( stopIfCeilingBlocked(collisionInfo) )
        {
            return;
        }

        if( collisionInfo.mid.floor.y <= core::DefaultCollisionRadius )
        {
            if( !tryStartSlide(collisionInfo) )
            {
                applyShift(collisionInfo);
                placeOnFloor(collisionInfo);
            }
            return;
        }

        setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
        setGoalAnimState( LaraStateId::JumpForward );
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = true;
    }
};
}
}
