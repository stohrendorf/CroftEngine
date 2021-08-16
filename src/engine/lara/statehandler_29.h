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

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)
       && getHandStatus() == objects::HandStatus::None)
    {
      setGoalAnimState(LaraStateId::Reach);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.floorCollisionRangeMin = core::HeightLimit;
    collisionInfo.floorCollisionRangeMax = -core::ClimbLimit2ClickMin;
    collisionInfo.ceilingCollisionRangeMin = 192_len;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraHangingHeight);
    checkJumpWallSmash(collisionInfo);
    if(collisionInfo.mid.floor.y > 0_len || getLara().m_state.fallspeed <= 0_spd)
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
} // namespace engine::lara
