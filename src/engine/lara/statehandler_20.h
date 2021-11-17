#pragma once

#include "statehandler_standing.h"

namespace engine::lara
{
class StateHandler_20 final : public StateHandler_Standing
{
public:
  explicit StateHandler_20(objects::LaraObject& lara)
      : StateHandler_Standing{lara, LaraStateId::TurnFast}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getYRotationSpeed() >= 0_deg / 1_frame)
    {
      setYRotationSpeed(core::FastTurnSpeed);
      if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
      {
        return;
      }
    }
    else
    {
      setYRotationSpeed(-core::FastTurnSpeed);
      if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
      {
        return;
      }
    }

    setGoalAnimState(LaraStateId::Stop);
  }
};
} // namespace engine::lara
