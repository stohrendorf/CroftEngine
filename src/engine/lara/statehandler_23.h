#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_23 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_23(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::RollBackward)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFalling(false);
                setFallSpeed(0);
                collisionInfo.facingAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls;
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;
                if( tryStartSlide(collisionInfo) )
                    return;

                if( collisionInfo.mid.floor.distance <= 200 )
                {
                    applyShift(collisionInfo);
                    placeOnFloor(collisionInfo);
                    return;
                }

                setAnimIdGlobal(loader::AnimationId::FREE_FALL_BACK, 1473);
                setTargetState(LaraStateId::FallBackward);
                setFallSpeed(0);
                setFalling(true);
            }
        };
    }
}
