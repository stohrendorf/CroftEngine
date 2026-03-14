#pragma once

#include "abstractstatehandler.h"
#include "statehandler_pushable.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_36 final : public StateHandler_Pushable
{
public:
  explicit StateHandler_36(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : StateHandler_Pushable{lara, LaraStateId::PushablePush}
  {
  }
};
} // namespace engine::lara