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

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
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

    getLara().m_state.fallspeed += -core::Gravity;
    if(getLara().m_state.fallspeed < core::Gravity * 1_frame)
    {
      getLara().m_state.fallspeed.stop();
    }
    if(getLara().m_state.fallspeed <= core::DeadlyHeadFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::UnderwaterStop);
    }
  }
};
} // namespace engine::lara
