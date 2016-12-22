#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_28 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_28(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::JumpUp)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            boost::optional<LaraStateId> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870

                if( auto nextHandler = tryGrabEdge(collisionInfo) )
                    return nextHandler;

                jumpAgainstWall(collisionInfo);
                if(getFallSpeed() < 0 || collisionInfo.current.floor.distance > 0)
                {
                    setTargetState(LaraStateId::JumpUp);
                    return{};
                }

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);
                setFalling(false);

                return {};
            }
        };
    }
}
