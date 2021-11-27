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
    collisionInfo.policies.reset(CollisionInfo::PolicyFlags::EnableSpaz);
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::EnableBaddiePush);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::SwandiveEnd);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);

    checkJumpWallSmash(collisionInfo);
    if(collisionInfo.mid.floor.y > 0_len || getLara().m_state.fallspeed <= 0_spd)
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
