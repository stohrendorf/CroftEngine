#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_25 final : public AbstractStateHandler
{
public:
  explicit StateHandler_25(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::JumpBack}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    getWorld().getCameraController().setRotationAroundLaraY(135_deg);
    if(getLara().m_state.fallspeed > core::FreeFallSpeedThreshold)
    {
      setGoalAnimState(LaraStateId::FreeFall);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 180_deg);
    commonJumpHandling(collisionInfo);
  }
};
} // namespace engine::lara
