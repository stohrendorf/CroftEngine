#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_8 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_8(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::Death)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.collisionRadius = 400;
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                applyCollisionFeedback(collisionInfo);
                placeOnFloor(collisionInfo);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                setHealth(-1);
                setAir(-1);
            }
        };
    }
}
