#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_55 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_55(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::OnWaterExit)
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
                collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
