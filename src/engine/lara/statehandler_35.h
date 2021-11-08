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

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      getLara().m_state.rotation.X -= toRenderUnit(1_deg / 1_frame) * 1_rframe;
    }
  }
};
} // namespace engine::lara
