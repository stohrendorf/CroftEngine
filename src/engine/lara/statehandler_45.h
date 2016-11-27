#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_45 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_45(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFalling(false);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant;
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                auto nextHandler = stopIfCeilingBlocked(collisionInfo);
                if( nextHandler )
                    return nextHandler;
                if( tryStartSlide(collisionInfo, nextHandler) )
                    return nextHandler;

                if( collisionInfo.current.floor.distance <= 200 )
                {
                    applyCollisionFeedback(collisionInfo);
                    placeOnFloor(collisionInfo);
                    return nextHandler;
                }

                playAnimation(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);

                return createWithRetainedAnimation(LaraStateId::JumpForward);
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::RollForward;
            }
        };
    }
}
