#pragma once

#include "statehandler_pushable.h"

namespace engine::lara
{
class StateHandler_36 final : public StateHandler_Pushable
{
public:
  explicit StateHandler_36(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Pushable{lara, LaraStateId::PushablePush}
  {
  }
};
} // namespace engine::lara
