#pragma once

#include "statehandler_onwater.h"

namespace engine::lara
{
class StateHandler_34 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_34(objects::LaraObject& lara)
      : StateHandler_OnWater{lara, LaraStateId::OnWaterForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.health <= 0_hp)
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    setSwimToDiveKeypressDuration(0_frame);

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= 4_deg;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += 4_deg;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().jump)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    getLara().m_state.fallspeed = std::min(60_spd, getLara().m_state.fallspeed + 8_spd);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
