#pragma once

#include "statehandler_pushable.h"

namespace engine::lara
{
class StateHandler_37 final : public StateHandler_Pushable
{
public:
  explicit StateHandler_37(const gsl::not_null<objects::LaraObject*>& lara)
      : StateHandler_Pushable{lara, LaraStateId::PushablePull}
  {
  }
};
} // namespace engine::lara
