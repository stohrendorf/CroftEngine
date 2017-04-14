#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/inputstate.h"
#include "level/level.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_3 final : public AbstractStateHandler
        {
        public:
            explicit StateHandler_3(LaraNode& lara)
                    : AbstractStateHandler(lara, LaraStateId::JumpForward)
            {
            }


            void handleInputImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& deltaTime) override
            {
                if( getTargetState() == LaraStateId::SwandiveBegin || getTargetState() == LaraStateId::Reach )
                    setTargetState(LaraStateId::JumpForward);

                if( getTargetState() != LaraStateId::Death && getTargetState() != LaraStateId::Stop )
                {
                    if(getLevel().m_inputHandler->getInputState().action && getHandStatus() == 0)
                        setTargetState(LaraStateId::Reach);

                    if(getLevel().m_inputHandler->getInputState().moveSlow && getHandStatus() == 0)
                        setTargetState(LaraStateId::SwandiveBegin);

                    if(getFallSpeed() > core::FreeFallSpeedThreshold)
                        setTargetState(LaraStateId::FreeFall);
                }

                if(getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left)
                {
                    subYRotationSpeed(deltaTime, 2.25_deg, -3_deg);
                }
                else if(getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right)
                {
                    addYRotationSpeed(deltaTime, 2.25_deg, 3_deg);
                }
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTime*/) override
            {
            }


            void postprocessFrame(CollisionInfo& collisionInfo, const std::chrono::microseconds& deltaTime) override
            {
                collisionInfo.passableFloorDistanceBottom = loader::HeightLimit;
                collisionInfo.passableFloorDistanceTop = -core::ClimbLimit2ClickMin;
                collisionInfo.neededCeilingDistance = 192;
                collisionInfo.yAngle = getRotation().Y;
                setMovementAngle(collisionInfo.yAngle);
                collisionInfo.initHeightInfo(getPosition(), getLevel(), core::ScalpHeight);
                checkJumpWallSmash(collisionInfo);

                if( collisionInfo.current.floor.distance >= -1 || getFallSpeed() < 0 )
                    return;

                if( applyLandingDamage() )
                {
                    setTargetState(LaraStateId::Death);
                }
                else if( getLevel().m_inputHandler->getInputState().zMovement != AxisMovement::Forward || getLevel().m_inputHandler->getInputState().moveSlow )
                {
                    setTargetState(LaraStateId::Stop);
                }
                else
                {
                    setTargetState(LaraStateId::RunForward);
                }

                setFallSpeed(core::makeInterpolatedValue(0.0f));
                setFalling(false);
                setHorizontalSpeed(core::makeInterpolatedValue(0.0f));
                placeOnFloor(collisionInfo);

                laraUpdateImpl(deltaTime);
            }
        };
    }
}
