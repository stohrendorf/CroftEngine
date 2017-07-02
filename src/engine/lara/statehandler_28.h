#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_28 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_28(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::JumpUp)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.badPositiveDistance = loader::HeightLimit;
                collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
                collisionInfo.badCeilingDistance = 192;
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

                if( tryGrabEdge(collisionInfo) )
                    return;

                jumpAgainstWall(collisionInfo);
                if( getFallSpeed() <= 0 || collisionInfo.mid.floor.distance > 0 )
                    return;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);
                setFallSpeed(0);
                placeOnFloor(collisionInfo);
                setFalling(false);
            }
        };
    }
}
