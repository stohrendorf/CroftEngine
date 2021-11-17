#pragma once

#include "statehandler_turnslow.h"

namespace engine::lara
{
class StateHandler_6 final : public StateHandler_TurnSlow
{
public:
  explicit StateHandler_6(objects::LaraObject& lara)
      : StateHandler_TurnSlow{lara, LaraStateId::TurnRightSlow}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    addYRotationSpeed(core::SlowTurnSpeedAcceleration);

    if(getHandStatus() == objects::HandStatus::Combat)
    {
      setGoalAnimState(LaraStateId::TurnFast);
    }
    else if(getYRotationSpeed() > core::SlowTurnSpeed)
    {
      if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
      {
        setYRotationSpeed(core::SlowTurnSpeed);
      }
      else
      {
        setGoalAnimState(LaraStateId::TurnFast);
      }
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      if(getWorld().getPresenter().getInputHandler().getInputState().xMovement != hid::AxisMovement::Right)
      {
        setGoalAnimState(LaraStateId::Stop);
      }
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
    {
      setGoalAnimState(LaraStateId::WalkForward);
    }
    else
    {
      setGoalAnimState(LaraStateId::RunForward);
    }
  }
};
} // namespace engine::lara
