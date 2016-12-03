#pragma once

#include "statehandler_pushable.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_36 final : public StateHandler_Pushable
        {
        public:
            explicit StateHandler_36(LaraNode& lara)
                    : StateHandler_Pushable(lara, LaraStateId::PushablePush)
            {
            }
        };
    }
}
