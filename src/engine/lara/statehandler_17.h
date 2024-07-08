#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "statehandler_underwater.h"

#include <algorithm>
#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_17 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_17(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterForward}
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

    if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::UnderwaterInertia);
    }

    getLara().m_state.fallspeed = std::min(getLara().m_state.fallspeed + 8_spd, 200_spd);
  }
};
} // namespace engine::lara
