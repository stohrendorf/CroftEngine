#pragma once

#include "statehandler_onwater.h"

namespace engine::lara
{
class StateHandler_49 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_49(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_OnWater{lara, LaraStateId::OnWaterRight}
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      getLara().m_state.rotation.Y += core::OnWaterMovementTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Left:
      getLara().m_state.rotation.Y -= core::OnWaterMovementTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Null:
      break;
    }

    if(inputHandler.getInputState().stepMovement != hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    getLara().m_state.fallspeed
      = std::min(core::OnWaterMaxSpeed, getLara().m_state.fallspeed + core::OnWaterAcceleration * 1_frame);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
