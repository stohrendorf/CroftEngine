#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_26 final : public AbstractStateHandler
{
public:
  explicit StateHandler_26(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpLeft}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    if(doPhysics)
      setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
    commonJumpHandling(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
