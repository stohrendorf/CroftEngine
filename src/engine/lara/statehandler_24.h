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

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                setCameraRotationX(-45_deg);
                if( getLevel().m_inputHandler->getInputState().jump )
                    setTargetState(LaraStateId::JumpForward);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y);
                return commonSlideHandling(collisionInfo);
            }
        };
    }
}
