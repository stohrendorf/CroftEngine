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
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    setSwimToDiveKeypressDuration(0_frame);

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= core::OnWaterTurnSpeed * 1_frame;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += core::OnWaterTurnSpeed * 1_frame;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    getLara().m_state.fallspeed
      = std::min(core::OnWaterMaxSpeed, getLara().m_state.fallspeed + core::OnWaterAcceleration * 1_frame);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
