#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "statehandler_turnslow.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_6 final : public StateHandler_TurnSlow
{
public:
  explicit StateHandler_6(const gsl::not_null<objects::LaraObject*>& lara)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(getHandStatus() == objects::HandStatus::Combat)
    {
      setGoalAnimState(LaraStateId::TurnFast);
    }
    else if(getYRotationSpeed() > core::SlowTurnSpeed)
    {
      if(inputHandler.hasAction(hid::Action::Walk))
      {
        setYRotationSpeed(core::SlowTurnSpeed);
      }
      else
      {
        setGoalAnimState(LaraStateId::TurnFast);
      }
    }

    if(inputHandler.getInputState().zMovement != hid::AxisMovement::Forward)
    {
      if(inputHandler.getInputState().xMovement != hid::AxisMovement::Right)
      {
        setGoalAnimState(LaraStateId::Stop);
      }
      return;
    }

    if(inputHandler.hasAction(hid::Action::Walk))
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
