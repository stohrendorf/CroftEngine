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


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() >= core::DeadlyFallSpeedThreshold )
                {
                    getLara().playSoundEffect(30);
                }
                dampenHorizontalSpeed(0.05f);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.facingAngle = getMovementAngle();
                setFalling(true);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                jumpAgainstWall(collisionInfo);
                if( collisionInfo.mid.floor.distance > 0 )
                    return;

                if( applyLandingDamage() )
                {
                    setTargetState(LaraStateId::Death);
                }
                else
                {
                    setTargetState(LaraStateId::Stop);
                    setAnimIdGlobal(loader::AnimationId::LANDING_HARD, 358);
                }
                getLevel().stopSoundEffect(30);
                setFallSpeed(0);
                placeOnFloor(collisionInfo);
                setFalling(false);
            }
        };
    }
}
