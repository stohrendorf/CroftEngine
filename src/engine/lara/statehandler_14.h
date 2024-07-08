#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "statehandler_standing.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_14 final : public StateHandler_Standing
{
public:
  explicit StateHandler_14(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Standing{lara, LaraStateId::GrabToFall}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }
};
} // namespace engine::lara
