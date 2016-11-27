#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_31 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_31(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                setCameraRotation(-60_deg, 0_deg);
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
                if( getLevel().m_inputHandler->getInputState().xMovement != AxisMovement::Right && getLevel().m_inputHandler->getInputState().stepMovement != AxisMovement::Right )
                    setTargetState(LaraStateId::Hang);

                return {};
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y + 90_deg);
                auto nextHandler = commonEdgeHangHandling(collisionInfo);
                setMovementAngle(getRotation().Y + 90_deg);
                return nextHandler;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::ShimmyRight;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }
        };
    }
}
