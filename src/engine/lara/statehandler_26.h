#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_26 final : public AbstractStateHandler
{
  public:
  explicit StateHandler_26(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpLeft}
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
    setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
    commonJumpHandling(collisionInfo);
  }
};
} // namespace lara
} // namespace engine
