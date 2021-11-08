#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_53 final : public AbstractStateHandler
{
public:
  explicit StateHandler_53(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SwandiveEnd}
  {
  }

  void handleInput(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    collisionInfo.policies.reset(CollisionInfo::PolicyFlags::EnableSpaz);
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::EnableBaddiePush);

    if(doPhysics)
      dampenHorizontalSpeed(0.05f);
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    if(!doPhysics)
      return;

    setMovementAngle(collisionInfo.facingAngle);

    checkJumpWallSmash(collisionInfo);
    if(collisionInfo.mid.floor.y > 0_len || getLara().m_state.fallspeed <= 0_spd)
    {
      return;
    }

    if(getLara().m_state.fallspeed <= core::DeadlyHeadFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::Stop);
    }
    else
    {
      setGoalAnimState(LaraStateId::Death);
    }

    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
