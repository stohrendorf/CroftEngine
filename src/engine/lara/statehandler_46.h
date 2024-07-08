#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/collisioninfo.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_46 final : public AbstractStateHandler
{
public:
  explicit StateHandler_46(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::BoulderDeath}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
    getWorld().getCameraController().setEyeRotation(-25_deg, 170_deg);
  }

  void postprocessFrame(CollisionInfo& /*collisionInfo*/) override
  {
  }
};
} // namespace engine::lara
