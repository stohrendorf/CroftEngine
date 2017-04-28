#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_26 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_26(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::JumpLeft)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
                else
                    setTargetState(LaraStateId::JumpLeft);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 90_deg);
                commonJumpHandling(collisionInfo);
            }
        };
    }
}
