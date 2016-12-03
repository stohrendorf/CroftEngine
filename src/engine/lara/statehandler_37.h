#pragma once

#include "statehandler_pushable.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_37 final : public StateHandler_Pushable
        {
        public:
            explicit StateHandler_37(LaraNode& lara)
                    : StateHandler_Pushable(lara, LaraStateId::PushablePull)
            {
            }
        };
    }
}
