#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "hid/actions.h"
#include "hid/inputstate.h"
#include "statehandler_onwater.h"

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_34 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_34(const gsl::not_null<objects::LaraObject*>& lara)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      getLara().m_state.rotation.Y += core::OnWaterTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Left:
      getLara().m_state.rotation.Y -= core::OnWaterTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Null:
      break;
    }

    if(inputHandler.getInputState().zMovement != hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::OnWaterStop);
    }

    if(inputHandler.hasAction(hid::Action::Jump))
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
