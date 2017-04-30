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


            void handleInput(CollisionInfo& collisionInfo) override
            {
                collisionInfo.policyFlags &= ~CollisionInfo::EnableSpaz;
                collisionInfo.policyFlags |= CollisionInfo::EnableBaddiePush;

                dampenHorizontalSpeed(0.05f);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                checkJumpWallSmash(collisionInfo);
                if( collisionInfo.mid.floor.distance > 0 || getFallSpeed() < 0 )
                    return;

                if( getFallSpeed() <= 133 )
                    setTargetState(LaraStateId::Stop);
                else
                    setTargetState(LaraStateId::Death);

                setFallSpeed(0);
                setFalling(false);
                placeOnFloor(collisionInfo);
            }
        };
    }
}
