#pragma once

#include "abstractstatehandler.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_22 final : public StateHandler_SideStep
{
public:
  explicit StateHandler_22(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : StateHandler_SideStep{lara, LaraStateId::StepLeft, hid::AxisMovement::Left, -90_deg}
  {
  }
};
} // namespace engine::lara