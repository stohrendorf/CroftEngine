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
    if(getLara().m_state.health <= 0_hp)
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    if(getYRotationSpeed() >= 0_deg)
    {
      setYRotationSpeed(8_deg);
      if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
      {
        return;
      }
    }
    else
    {
      setYRotationSpeed(-8_deg);
      if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
      {
        return;
      }
    }

    setGoalAnimState(LaraStateId::Stop);
  }
};
} // namespace engine::lara
