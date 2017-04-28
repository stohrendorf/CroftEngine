#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_52 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_52(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::SwandiveBegin)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                collisionInfo.policyFlags &= ~CollisionInfo::EnableSpaz;
                collisionInfo.policyFlags |= CollisionInfo::EnableBaddiePush;
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::SwandiveEnd);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
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

                setTargetState(LaraStateId::Stop);
                setFallSpeed(0);
                setFalling(false);
                placeOnFloor(collisionInfo);
            }
        };
    }
}
