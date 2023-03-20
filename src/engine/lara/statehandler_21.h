#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"
#include "statehandler_sidestep.h"

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
