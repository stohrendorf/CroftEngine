#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_19 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_19(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::Climbing)
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
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
