#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_28 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_28(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);

                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

                if( auto nextHandler = tryGrabEdge(collisionInfo) )
                    return nextHandler;

                jumpAgainstWall(collisionInfo);
                if( getFallSpeed() <= 0 || collisionInfo.current.floor.distance > 0 )
                    return nullptr;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);
                setFalling(false);

                return nullptr;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::JumpUp;
            }
        };
    }
}
