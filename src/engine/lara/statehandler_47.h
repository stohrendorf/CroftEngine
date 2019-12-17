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

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.health <= 0_hp)
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    setSwimToDiveKeypressDuration(0_frame);

    if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= 2_deg;
    }
    else if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += 2_deg;
    }

    if(getEngine().getInputHandler().getInputState().zMovement != hid::AxisMovement::Backward)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    getLara().m_state.fallspeed = std::min(60_spd, getLara().m_state.fallspeed + 8_spd);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
