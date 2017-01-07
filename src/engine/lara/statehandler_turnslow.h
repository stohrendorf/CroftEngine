#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_TurnSlow : public AbstractStateHandler
        {
        protected:
            explicit StateHandler_TurnSlow(LaraNode& lara, LaraStateId id)
                    : AbstractStateHandler(lara, id)
            {
            }

        public:
            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.passableFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.policyFlags |= CollisionInfo::SlopesAreWalls | CollisionInfo::SlopesArePits;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( collisionInfo.current.floor.distance <= 100 )
                {
                    boost::optional<LaraStateId> nextHandler;
                    if( !tryStartSlide(collisionInfo, nextHandler) )
                        placeOnFloor(collisionInfo);

                    return nextHandler;
                }

                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);
                return LaraStateId::JumpForward;
            }
        };
    }
}
