#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "hid/actions.h"
#include "statehandler_underwater.h"

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_18 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_18(const gsl::not_null<objects::LaraObject*>& lara)
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
