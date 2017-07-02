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
                setCameraOldMode(CameraMode::Fixed);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
                collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
                collisionInfo.badCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
