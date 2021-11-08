#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_25 final : public AbstractStateHandler
{
public:
  explicit StateHandler_25(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpBack}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    getWorld().getCameraController().setRotationAroundLaraY(135_deg);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    if(doPhysics)
      setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    
    commonJumpHandling(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
