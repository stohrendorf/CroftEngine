#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_9 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_9(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                dampenHorizontalSpeed(deltaTime, 0.05f);
                if( getFallSpeed() > 154 )
                {
                    getController().playSoundEffect(30);
                }
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getMovementAngle();
                setFalling(true);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                jumpAgainstWall(collisionInfo);
                if( collisionInfo.current.floor.distance > 0 )
                    return nullptr;

                std::unique_ptr<AbstractStateHandler> nextHandler = nullptr;
                if( applyLandingDamage() )
                {
                    setTargetState(LaraStateId::Death);
                }
                else
                {
                    setTargetState(LaraStateId::Stop);
                    nextHandler = createWithRetainedAnimation(LaraStateId::Stop);
                    playAnimation(loader::AnimationId::LANDING_HARD, 358);
                }
                getLevel().stopSoundEffect(30);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);
                setFalling(false);

                return nextHandler;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::FreeFall;
            }
        };
    }
}
