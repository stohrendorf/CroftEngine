#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_29 final : public AbstractStateHandler
{
public:
  explicit StateHandler_29(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::FallBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }

    if(getEngine().getInputHandler().getInputState().action && getHandStatus() == objects::HandStatus::None)
    {
      setGoalAnimState(LaraStateId::Reach);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraHangingHeight);
    checkJumpWallSmash(collisionInfo);
    if(collisionInfo.mid.floorSpace.y > 0_len || getLara().m_state.fallspeed <= 0_spd)
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
    placeOnFloor(collisionInfo);
    getLara().m_state.falling = false;
  }
};
} // namespace lara
