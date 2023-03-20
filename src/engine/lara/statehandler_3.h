#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_3 final : public AbstractStateHandler
{
public:
  explicit StateHandler_3(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getGoalAnimState() == LaraStateId::SwandiveBegin || getGoalAnimState() == LaraStateId::Reach)
    {
      setGoalAnimState(LaraStateId::JumpForward);
    }

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(getGoalAnimState() != LaraStateId::Death && getGoalAnimState() != LaraStateId::Stop)
    {
      if(inputHandler.hasAction(hid::Action::Action) && getHandStatus() == objects::HandStatus::None)
      {
        setGoalAnimState(LaraStateId::Reach);
      }

      if(inputHandler.hasAction(hid::Action::Walk) && getHandStatus() == objects::HandStatus::None)
      {
        setGoalAnimState(LaraStateId::SwandiveBegin);
      }

      if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
      {
        setGoalAnimState(LaraStateId::FreeFall);
      }
    }

    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      addYRotationSpeed(core::SlowTurnSpeedAcceleration, core::JumpTurnSpeed);
      break;
    case hid::AxisMovement::Left:
      subYRotationSpeed(core::SlowTurnSpeedAcceleration, -core::JumpTurnSpeed);
      break;
    case hid::AxisMovement::Null:
      break;
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

    if(applyLandingDamage())
    {
      setGoalAnimState(LaraStateId::Death);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward
            || getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
    {
      setGoalAnimState(LaraStateId::Stop);
    }
    else
    {
      setGoalAnimState(LaraStateId::RunForward);
    }

    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    getLara().m_state.speed = 0_spd;
    placeOnFloor(collisionInfo);

    laraAdvanceFrame();
  }
};
} // namespace engine::lara
