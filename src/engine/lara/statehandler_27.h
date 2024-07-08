#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/collisioninfo.h"

#include <core/magic.h>
#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_27 final : public AbstractStateHandler
{
public:
  explicit StateHandler_27(const gsl::not_null<objects::LaraObject*>& lara)
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
} // namespace engine::lara
