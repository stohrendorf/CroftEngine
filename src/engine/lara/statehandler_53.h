#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_53 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_53(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SwandiveEnd)
            {
            }


            void handleInputImpl(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                collisionInfo.policyFlags &= ~CollisionInfo::EnableSpaz;
                collisionInfo.policyFlags |= CollisionInfo::EnableBaddiePush;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                dampenHorizontalSpeed(deltaTime, 0.05f);
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                checkJumpWallSmash(collisionInfo);
                if( collisionInfo.current.floor.distance > 0 || getFallSpeed() < 0 )
                    return;

                if( getFallSpeed() <= 133 )
                    setTargetState(LaraStateId::Stop);
                else
                    setTargetState(LaraStateId::Death);

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                placeOnFloor(collisionInfo);
            }
        };
    }
}
