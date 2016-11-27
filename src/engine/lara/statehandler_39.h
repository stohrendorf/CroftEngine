#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_38 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_38(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
                setCameraRotationY(75_deg);
                if(!getLevel().m_inputHandler->getInputState().action)
                    setTargetState(LaraStateId::Stop);
                else
                    setTargetState(LaraStateId::PushableGrab);
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::PushableGrab;
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnwalkableSteepFloor | CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                return nullptr;
            }
        };
    }
}
