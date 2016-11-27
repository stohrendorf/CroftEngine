#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_8 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_8(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                collisionInfo.frobbelFlags &= ~(CollisionInfo::FrobbelFlag08 | CollisionInfo::FrobbelFlag10);
                return {};
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.collisionRadius = 400;
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                applyCollisionFeedback(collisionInfo);
                placeOnFloor(collisionInfo);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                setHealth(core::makeInterpolatedValue(-1.0f));
                //! @todo set air=-1
                return {};
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::Death;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }
        };
    }
}
