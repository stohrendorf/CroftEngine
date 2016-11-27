#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_29 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_29(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                    setTargetState(LaraStateId::FreeFall);

                if( getLevel().m_inputHandler->getInputState().action && getHandStatus() == 0 )
                    setTargetState(LaraStateId::Reach);

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
                collisionInfo.yAngle = getRotation().Y + 180_deg;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), 870); //! @todo MAGICK 870
                auto nextHandler = checkJumpWallSmash(collisionInfo);
                if( collisionInfo.current.floor.distance > 0 || getFallSpeed() <= 0 )
                    return nextHandler;

                if( applyLandingDamage() )
                    setTargetState(LaraStateId::Death);
                else
                    setTargetState(LaraStateId::Stop);

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);
                setFalling(false);

                return nextHandler;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::FallBackward;
            }
        };
    }
}
