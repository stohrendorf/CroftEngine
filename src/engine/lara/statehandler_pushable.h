#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_Pushable : public AbstractStateHandler
        {
        public:
            explicit StateHandler_Pushable(LaraNode& lara, LaraStateId id)
                : AbstractStateHandler(lara, id)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override final
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableBaddiePush | CollisionInfo::EnableSpaz);
                setCameraUnknown1(CamOverrideType::NotActivatedByLara);
                setCameraRotation(-25_deg, 35_deg);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
