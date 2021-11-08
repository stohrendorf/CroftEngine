#pragma once

#include "statehandler_onwater.h"

namespace engine::lara
{
class StateHandler_47 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_47(objects::LaraObject& lara)
      : StateHandler_OnWater{lara, LaraStateId::OnWaterBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    setSwimToDiveKeypressDuration(0_rframe);

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= core::OnWaterMovementTurnSpeed * 1_rframe;
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += core::OnWaterMovementTurnSpeed * 1_rframe;
    }

    if(inputHandler.getInputState().zMovement != hid::AxisMovement::Backward)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    getLara().m_state.fallspeed += core::OnWaterAcceleration;
    if(getLara().m_state.fallspeed > core::OnWaterMaxSpeed)
      getLara().m_state.fallspeed.stop(core::OnWaterMaxSpeed);
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    if(doPhysics)
      setMovementAngle(getLara().m_state.rotation.Y + 180_deg);

    commonOnWaterHandling(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
