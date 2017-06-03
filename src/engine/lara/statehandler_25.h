#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_25 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_25(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::JumpBack)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                setCameraCurrentRotationY(135_deg);
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 180_deg);
                commonJumpHandling(collisionInfo);
            }
        };
    }
}
