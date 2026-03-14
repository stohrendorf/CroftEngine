#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"
#include "statehandler_standing.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_4 final : public StateHandler_Standing
{
public:
  explicit StateHandler_4(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : StateHandler_Standing{lara, LaraStateId::Pose}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }
};
} // namespace engine::lara