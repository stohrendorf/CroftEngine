#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_5 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_5(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                setTargetState(LaraStateId::Stop);
                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    subYRotationSpeed(deltaTime, 2.25_deg, -6_deg);
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    addYRotationSpeed(deltaTime, 2.25_deg, 6_deg);
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.frobbelFlags |= CollisionInfo::FrobbelFlag_UnpassableSteepUpslant | CollisionInfo::FrobbelFlag_UnwalkableSteepFloor;
                collisionInfo.yAngle = getRotation().Y + 180_deg;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                if( auto nextHandler = stopIfCeilingBlocked(collisionInfo) )
                    return nextHandler;

                if( collisionInfo.current.floor.distance > 200 )
                {
                    playAnimation(loader::AnimationId::FREE_FALL_BACK, 1473);
                    setTargetState(LaraStateId::FallBackward);
                    setFallSpeed(core::makeInterpolatedValue(0.0f));
                    setFalling(true);
                    return createWithRetainedAnimation(LaraStateId::FallBackward);
                }

                auto nextHandler = checkWallCollision(collisionInfo);
                if( nextHandler )
                {
                    playAnimation(loader::AnimationId::STAY_SOLID, 185);
                }
                placeOnFloor(collisionInfo);

                return nextHandler;
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::RunBack;
            }
        };
    }
}
