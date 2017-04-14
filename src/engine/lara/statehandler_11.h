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


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                setCameraRotationY(85_deg);
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setFalling(true);
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = 0;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if(tryReach(collisionInfo))
                    return;

                jumpAgainstWall(collisionInfo);
                if( getFallSpeed() < 0 || collisionInfo.current.floor.distance > 0 )
                    return;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                placeOnFloor(collisionInfo);
            }
        };
    }
}
