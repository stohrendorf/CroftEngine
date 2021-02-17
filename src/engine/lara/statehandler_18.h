#pragma once

#include "statehandler_underwater.h"

namespace engine::lara
{
class StateHandler_18 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_18(objects::LaraObject& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterInertia}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    handleDiveRotationInput();

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::UnderwaterForward);
    }

    getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - core::Gravity * 1_frame);
    if(getLara().m_state.fallspeed <= core::DeadlyHeadFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::UnderwaterStop);
    }
  }
};
} // namespace engine::lara
