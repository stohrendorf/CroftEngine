#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_27 final : public AbstractStateHandler
{
public:
  explicit StateHandler_27(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpRight}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
    commonJumpHandling(collisionInfo);
  }
};
} // namespace lara
