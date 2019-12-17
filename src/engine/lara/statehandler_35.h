#pragma once

#include "statehandler_underwater.h"

namespace engine::lara
{
class StateHandler_35 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_35(objects::LaraObject& lara)
      : StateHandler_Underwater{lara, LaraStateId::UnderwaterDiving}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      getLara().m_state.rotation.X -= 1_deg;
    }
  }
};
} // namespace engine::lara
