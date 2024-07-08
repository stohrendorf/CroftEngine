#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "hid/inputstate.h"
#include "statehandler_sidestep.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_21 final : public StateHandler_SideStep
{
public:
  explicit StateHandler_21(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_SideStep{lara, LaraStateId::StepRight, hid::AxisMovement::Right, 90_deg}
  {
  }
};
} // namespace engine::lara
