#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_Standing : public AbstractStateHandler
        {
        protected:
            explicit StateHandler_Standing(LaraNode& lara, LaraStateId id)
                : AbstractStateHandler(lara, id)
            {
            }


        public:
            void postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setFallSpeed(0);
                setFalling(false);
                collisionInfo.facingAngle = getRotation().Y;
                setMovementAngle(collisionInfo.facingAngle);
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( stopIfCeilingBlocked(collisionInfo) )
                    return;

                if( collisionInfo.mid.floor.distance <= 100 )
                {
                    if( !tryStartSlide(collisionInfo) )
                    {
                        applyCollisionFeedback(collisionInfo);
                        placeOnFloor(collisionInfo);
                    }
                    return;
                }

                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(0);
                setFalling(true);
            }
        };
    }
}
