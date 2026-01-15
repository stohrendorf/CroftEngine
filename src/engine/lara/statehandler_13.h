#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/objects/laraobject.h"
#include "hid/actions.h"
#include "statehandler_underwater.h"

#include <algorithm>
#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_13 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_13(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterStop}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    handleDiveRotationInput(inputHandler.getInputState().xMovement, inputHandler.getInputState().zMovement);

    if(inputHandler.hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::UnderwaterForward);
    }

    getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - core::Gravity * 1_frame);
  }
};
} // namespace engine::lara