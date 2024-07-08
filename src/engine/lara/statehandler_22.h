#pragma once

#include "abstractstatehandler.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_22 final : public StateHandler_SideStep
{
public:
  explicit StateHandler_22(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_SideStep{lara, LaraStateId::StepLeft, hid::AxisMovement::Left, -90_deg}
  {
  }
};
} // namespace engine::lara
