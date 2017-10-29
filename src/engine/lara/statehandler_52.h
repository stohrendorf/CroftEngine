#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_52 final
    : public AbstractStateHandler
{
public:
    explicit StateHandler_52(LaraNode& lara)
        : AbstractStateHandler(lara, LaraStateId::SwandiveBegin)
    {
    }

    void handleInput(CollisionInfo& collisionInfo) override
    {
        collisionInfo.policyFlags &= ~CollisionInfo::EnableSpaz;
        collisionInfo.policyFlags |= CollisionInfo::EnableBaddiePush;
        if( getLara().m_state.fallspeed > core::FreeFallSpeedThreshold )
        {
            setTargetState(LaraStateId::SwandiveEnd);
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
        if( collisionInfo.mid.floor.distance > 0 || getLara().m_state.fallspeed <= 0 )
        {
            return;
        }

        setTargetState(LaraStateId::Stop);
        getLara().m_state.fallspeed = 0;
        getLara().m_state.falling = false;
        placeOnFloor(collisionInfo);
    }
};
}
}
