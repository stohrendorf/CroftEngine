#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_Standing : public AbstractStateHandler
        {
        protected:
            explicit StateHandler_Standing(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

        public:
            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override final
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededFloorDistanceBottom = core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                auto nextHandler = stopIfCeilingBlocked(collisionInfo);
                if( nextHandler )
                    return nextHandler;

                if( collisionInfo.current.floor.distance <= 100 )
                {
                    if( !tryStartSlide(collisionInfo, nextHandler) )
                    {
                        applyCollisionFeedback(collisionInfo);
                        placeOnFloor(collisionInfo);
                    }
                    return nextHandler;
                }

                setAnimIdGlobal(loader::AnimationId::FREE_FALL_FORWARD, 492);
                setTargetState(LaraStateId::JumpForward);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(true);
                return createWithRetainedAnimation(LaraStateId::JumpForward);
            }
        };
    }
}
