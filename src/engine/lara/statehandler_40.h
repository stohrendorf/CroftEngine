#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_40 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_40(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SwitchDown)
            {
            }


            void handleInputImpl(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                collisionInfo.policyFlags &= ~(CollisionInfo::EnableSpaz | CollisionInfo::EnableBaddiePush);
                setCameraRotation(-25_deg, 80_deg);
                setCameraDistance(1024);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                setMovementAngle(getRotation().Y);
                collisionInfo.yAngle = getRotation().Y;
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesArePits | CollisionInfo::SlopesAreWalls;

                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
            }
        };
    }
}
