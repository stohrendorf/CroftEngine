#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_11 final : public AbstractStateHandler
{
public:
  explicit StateHandler_11(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::Reach}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    setCameraRotationAroundLaraY(85_deg);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.falling = true;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.validFloorHeight = {0_len, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    if(tryReach(collisionInfo))
    {
      return;
    }

    jumpAgainstWall(collisionInfo);
    if(getLara().m_state.fallspeed <= 0_spd || collisionInfo.mid.floor.y > 0_len)
    {
      return;
    }

    if(applyLandingDamage())
    {
      setGoalAnimState(LaraStateId::Death);
    }
    else
    {
      setGoalAnimState(LaraStateId::Stop);
    }

    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
