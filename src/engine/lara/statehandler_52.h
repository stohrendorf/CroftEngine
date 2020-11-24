#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_52 final : public AbstractStateHandler
{
public:
  explicit StateHandler_52(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SwandiveBegin}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policyFlags.reset(CollisionInfo::PolicyFlags::EnableSpaz);
    collisionInfo.policyFlags.set(CollisionInfo::PolicyFlags::EnableBaddiePush);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::SwandiveEnd);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);
    checkJumpWallSmash(collisionInfo);
    if(collisionInfo.mid.floorSpace.y > 0_len || getLara().m_state.fallspeed <= 0_spd)
    {
      return;
    }

    setGoalAnimState(LaraStateId::Stop);
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
