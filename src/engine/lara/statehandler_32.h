#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_32 final : public AbstractStateHandler
{
public:
  explicit StateHandler_32(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::SlideBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::JumpBack);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    commonSlideHandling(collisionInfo);
  }
};
} // namespace engine::lara
