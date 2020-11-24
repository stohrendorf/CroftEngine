#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_24 final : public AbstractStateHandler
{
public:
  explicit StateHandler_24(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SlideForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    setCameraModifier(CameraModifier::AllowSteepSlants);
    setCameraRotationAroundLaraX(-45_deg);
    if(getWorld().getPresenter().getInputHandler().getInputState().jump)
    {
      setGoalAnimState(LaraStateId::JumpForward);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonSlideHandling(collisionInfo);
  }
};
} // namespace engine::lara
