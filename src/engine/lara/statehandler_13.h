#pragma once

#include "statehandler_underwater.h"

namespace engine::lara
{
class StateHandler_13 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_13(objects::LaraObject& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterStop}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool doPhysics) override
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
    if(doPhysics && getLara().m_state.fallspeed <= core::Gravity * 1_frame)
    {
      getLara().m_state.fallspeed.stop();
    }
  }
};
} // namespace engine::lara
