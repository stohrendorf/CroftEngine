#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_45 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_45(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::RollForward)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFalling(false);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls;
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if(stopIfCeilingBlocked(collisionInfo))
                    return;

                if( tryStartSlide(collisionInfo) )
                    return;

                if( collisionInfo.current.floor.distance <= 200 )
                {
                    applyCollisionFeedback(collisionInfo);
                    placeOnFloor(collisionInfo);
                    return;
                }

                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);
            }
        };
    }
}
