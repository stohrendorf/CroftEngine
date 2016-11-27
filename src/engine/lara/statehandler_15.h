#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_15 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_15(LaraNode& lara)
                    : AbstractStateHandler(lara)
            {
            }

            std::unique_ptr<AbstractStateHandler> handleInputImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward && getRelativeHeightAtDirection(getRotation().Y, 256) >= -core::ClimbLimit2ClickMin )
                {
                    setMovementAngle(getRotation().Y);
                    setTargetState(LaraStateId::JumpForward);
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left && getRelativeHeightAtDirection(getRotation().Y - 90_deg, 256) >= -core::ClimbLimit2ClickMin )
                {
                    setMovementAngle(getRotation().Y - 90_deg);
                    setTargetState(LaraStateId::JumpRight);
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right && getRelativeHeightAtDirection(getRotation().Y + 90_deg, 256) >= -core::ClimbLimit2ClickMin )
                {
                    setMovementAngle(getRotation().Y + 90_deg);
                    setTargetState(LaraStateId::JumpLeft);
                }
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward && getRelativeHeightAtDirection(getRotation().Y + 180_deg, 256) >= -core::ClimbLimit2ClickMin )
                {
                    setMovementAngle(getRotation().Y + 180_deg);
                    setTargetState(LaraStateId::JumpBack);
                }

                if( getFallSpeed() > core::FreeFallSpeedThreshold )
                {
                    setTargetState(LaraStateId::FreeFall);
                }

                return nullptr;
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
            }

            std::unique_ptr<AbstractStateHandler> postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                collisionInfo.neededFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.neededFloorDistanceTop = -loader::HeightLimit;
                collisionInfo.neededCeilingDistance = 0;
                collisionInfo.yAngle = getRotation().Y;
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);

                if( collisionInfo.current.ceiling.distance <= -100 )
                    return nullptr;

                setTargetState(LaraStateId::Stop);
                playAnimation(loader::AnimationId::STAY_SOLID, 185);
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                setPosition(collisionInfo.position);

                return createWithRetainedAnimation(LaraStateId::Stop);
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::JumpPrepare;
            }
        };
    }
}
