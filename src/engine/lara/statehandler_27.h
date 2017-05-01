#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_27 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_27(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::JumpRight)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y - 90_deg);
                commonJumpHandling(collisionInfo);
            }
        };
    }
}
