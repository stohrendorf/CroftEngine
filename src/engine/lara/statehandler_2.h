#pragma once

#include "statehandler_standing.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_2 final : public StateHandler_Standing
        {
        public:
            explicit StateHandler_2(LaraNode& lara)
                    : StateHandler_Standing(lara)
            {
            }

            boost::optional<LaraStateId> handleInputImpl(CollisionInfo& collisionInfo) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Death);
                    return {};
                }

                if( getLevel().m_inputHandler->getInputState().roll )
                {
                    setAnimIdGlobal(loader::AnimationId::ROLL_BEGIN);
                    setTargetState(LaraStateId::Stop);
                    return LaraStateId::RollForward;
                }

                setTargetState(LaraStateId::Stop);

                if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
                {
                    setTargetState(LaraStateId::StepLeft);
                }
                else if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
                {
                    setTargetState(LaraStateId::StepRight);
                }

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                {
                    setTargetState(LaraStateId::TurnLeftSlow);
                }
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                {
                    setTargetState(LaraStateId::TurnRightSlow);
                }

                if( getLevel().m_inputHandler->getInputState().jump )
                {
                    setTargetState(LaraStateId::JumpPrepare);
                }
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                {
                    if( getLevel().m_inputHandler->getInputState().moveSlow )
                        create(LaraStateId::WalkForward, getLara())->handleInputImpl(collisionInfo);
                    else
                        create(LaraStateId::RunForward, getLara())->handleInputImpl(collisionInfo);
                }
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                {
                    if( getLevel().m_inputHandler->getInputState().moveSlow )
                        create(LaraStateId::WalkBackward, getLara())->handleInputImpl(collisionInfo);
                    else
                        setTargetState(LaraStateId::RunBack);
                }

                return {};
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/, const std::chrono::microseconds& /*deltaTimeMs*/) override
            {
                if(getLevel().m_inputHandler->getInputState().freeLook)
                {
                    getLevel().m_cameraController->setCamOverrideType(2);
                    getLevel().m_cameraController->addHeadRotationXY(
                            -FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.y/2000),
                            FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.x/2000)
                    );
                    auto r = getLevel().m_cameraController->getHeadRotation();
                    if(r.Y < -44_deg)
                        r.Y = -44_deg;
                    else if(r.Y > 44_deg)
                        r.Y = 44_deg;

                    if(r.X < -42_deg)
                        r.X = -42_deg;
                    else if(r.X > 22_deg)
                        r.X = 22_deg;

                    getLevel().m_cameraController->setHeadRotation(r);
                    getLevel().m_cameraController->setTorsoRotation(r);
                }
                else if(getLevel().m_cameraController->getCamOverrideType() == 2)
                {
                    getLevel().m_cameraController->setCamOverrideType(0);
                }
            }

            loader::LaraStateId getId() const noexcept override
            {
                return LaraStateId::Stop;
            }
        };
    }
}
