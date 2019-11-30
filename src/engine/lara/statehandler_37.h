#pragma once

#include "statehandler_pushable.h"

namespace engine::lara
{
class StateHandler_37 final : public StateHandler_Pushable
{
public:
  explicit StateHandler_37(objects::LaraObject& lara)
      : StateHandler_Pushable{lara, LaraStateId::PushablePull}
  {
  }
};
} // namespace lara
