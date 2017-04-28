#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_39 final : public AbstractStateHandler
        {
        public:

            explicit StateHandler_39(LaraNode& lara)
                : AbstractStateHandler(lara, LaraStateId::PickUp)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                setCameraRotation(15_deg, 130_deg);
                setCameraDistance(1024);
            }


            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
