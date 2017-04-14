#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_24 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_24(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SlideForward)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                setCameraRotationX(-45_deg);
                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::JumpForward);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setMovementAngle(getRotation().Y);
                commonSlideHandling(collisionInfo);
            }
        };
    }
}
