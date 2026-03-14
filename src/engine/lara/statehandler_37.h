#pragma once

#include "abstractstatehandler.h"
#include "engine/objects/laraobject.h"
#include "statehandler_pushable.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_37 final : public StateHandler_Pushable
{
public:
  explicit StateHandler_37(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : StateHandler_Pushable{lara, LaraStateId::PushablePull}
  {
  }
};
} // namespace engine::lara