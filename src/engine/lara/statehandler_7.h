#pragma once

#include "statehandler_turnslow.h"

namespace engine
{
namespace lara
{
class StateHandler_7 final : public StateHandler_TurnSlow
{
  public:
  explicit StateHandler_7(LaraNode& lara)
      : StateHandler_TurnSlow{lara, LaraStateId::TurnLeftSlow}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.health <= 0_hp)
    {
      setGoalAnimState(LaraStateId::Stop);
      return;
    }

    subYRotationSpeed(2.25_deg);

    if(getHandStatus() == HandStatus::Combat)
    {
      setGoalAnimState(LaraStateId::TurnFast);
    }
    else if(getYRotationSpeed() < -4_deg)
    {
      if(getEngine().getInputHandler().getInputState().moveSlow)
      {
        setYRotationSpeed(-4_deg);
      }
      else
      {
        setGoalAnimState(LaraStateId::TurnFast);
      }
    }

    if(getEngine().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      if(getEngine().getInputHandler().getInputState().xMovement != hid::AxisMovement::Left)
      {
        setGoalAnimState(LaraStateId::Stop);
      }
      return;
    }

    if(getEngine().getInputHandler().getInputState().moveSlow)
    {
      setGoalAnimState(LaraStateId::WalkForward);
    }
    else
    {
      setGoalAnimState(LaraStateId::RunForward);
    }
  }
};
} // namespace lara
} // namespace engine
