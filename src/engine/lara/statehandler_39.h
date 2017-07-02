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
                setCameraCurrentRotation(-15_deg, -130_deg);
                setCameraTargetDistance(1024);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
                collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
                collisionInfo.badCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
