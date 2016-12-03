#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_11 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_11(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::Reach)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                setCameraRotationY(85_deg);
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFalling(true);
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = 0;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                auto nextHandler = tryReach(collisionInfo);
                if( nextHandler )
                    return nextHandler;

                jumpAgainstWall(collisionInfo);
                if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
                    return nextHandler;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                placeOnFloor(collisionInfo);

                return nextHandler;
            }
        };
    }
}
