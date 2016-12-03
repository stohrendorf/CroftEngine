#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
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
                    : AbstractStateHandler(lara, LaraStateId::FreeFall)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                dampenHorizontalSpeed(deltaTime, 0.05f);
                if( getFallSpeed() > 154 )
                {
                    getLara().playSoundEffect(30);
                }
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getMovementAngle();
                setFalling(true);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                jumpAgainstWall(collisionInfo);
                if( collisionInfo.current.floor.distance > 0 )
                    return {};

                boost::optional<LaraStateId> nextHandler;
                if( applyLandingDamage() )
                {
                    setTargetState(LaraStateId::Death);
                }
                else
                {
                    setTargetState(LaraStateId::Stop);
                    nextHandler = LaraStateId::Stop;
                    setAnimIdGlobal(loader::AnimationId::LANDING_HARD, 358);
                }
                getLevel().stopSoundEffect(30);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);
                setFalling(false);

                return nextHandler;
            }
        };
    }
}
