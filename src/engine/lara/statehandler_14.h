#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
    class StateHandler_14 final : public StateHandler_Standing
    {
    public:
        explicit StateHandler_14(LaraNode& lara)
            : StateHandler_Standing(lara, LaraStateId::GrabToFall)
        {
        }


        void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
        {
        }

        void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
        {
        }
    };
    }
}
