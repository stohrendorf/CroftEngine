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

            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

                if( tryGrabEdge(collisionInfo) )
                    return;

                jumpAgainstWall(collisionInfo);
                if(getFallSpeed() < 0 || collisionInfo.current.floor.distance > 0)
                {
                    setTargetState(LaraStateId::JumpUp);
                    return;
                }

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);
                setFallSpeed(0);
                placeOnFloor(collisionInfo);
                setFalling(false);

                return;
            }
        };
    }
}
