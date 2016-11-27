#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_53 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_53(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                collisionInfo.frobbelFlags &= ~CollisionInfo::FrobbelFlag10;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag08;
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                dampenHorizontalSpeed(deltaTime, 0.05f);
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                auto nextHandler = checkJumpWallSmash(collisionInfo);
                if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                    return nextHandler;

                if( getFallSpeed() <= 133 )
                    setTargetState(LaraStateId::Stop);
                else
                    setTargetState(LaraStateId::Death);

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                placeOnFloor(collisionInfo);

                return nextHandler;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::SwandiveEnd;
            }
        };
    }
}
