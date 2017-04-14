#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_32 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_32(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SlideBackward)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::JumpBack);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setMovementAngle(getRotation().Y + 180_deg);
                commonSlideHandling(collisionInfo);
            }
        };
    }
}
