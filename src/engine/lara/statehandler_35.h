#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "hid/inputstate.h"
#include "statehandler_underwater.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_35 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_35(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterDiving}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      getLara().m_state.rotation.X -= 1_deg;
    }
  }
};
} // namespace engine::lara
