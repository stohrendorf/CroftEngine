#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_22 final : public StateHandler_SideStep
{
public:
  explicit StateHandler_22(objects::LaraObject& lara)
      : StateHandler_SideStep{lara, LaraStateId::StepLeft, hid::AxisMovement::Left, -90_deg}
  {
  }
};
} // namespace engine::lara
