#pragma once

#include "statehandler_standing.h"

namespace engine::lara
{
class StateHandler_4 final : public StateHandler_Standing
{
public:
  explicit StateHandler_4(objects::LaraObject& lara)
      : StateHandler_Standing{lara, LaraStateId::Pose}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }
};
} // namespace engine::lara
