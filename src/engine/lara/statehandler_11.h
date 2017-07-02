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


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                setCameraCurrentRotationY(85_deg);
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFalling(true);
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.badPositiveDistance = loader::HeightLimit;
                collisionInfo.badNegativeDistance = 0;
                collisionInfo.badCeilingDistance = 192;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( tryReach(collisionInfo) )
                    return;

                jumpAgainstWall(collisionInfo);
                if( getFallSpeed() <= 0 || collisionInfo.mid.floor.distance > 0 )
                    return;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);

                setFallSpeed(0);
                setFalling(false);
                placeOnFloor(collisionInfo);
            }
        };
    }
}
