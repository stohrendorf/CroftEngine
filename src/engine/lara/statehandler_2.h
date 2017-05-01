#pragma once

#include "statehandler_standing.h"
#include "engine/cameracontroller.h"


namespace engine
{
    namespace lara
    {
        class StateHandler_2 final : public StateHandler_Standing
        {
        public:
            explicit StateHandler_2(LaraNode& lara)
                : StateHandler_Standing(lara, LaraStateId::Stop)
            {
            }


            void handleInput(CollisionInfo& collisionInfo) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::Death);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().roll )
                {
                    setAnimIdGlobal(loader::AnimationId::ROLL_BEGIN);
                    setTargetState(LaraStateId::Stop);
                    return;
                }

                setTargetState(LaraStateId::Stop);

                if( getLevel().m_inputHandler->getInputState().freeLook )
                {
                    getLevel().m_cameraController->setCamOverrideType(CamOverrideType::FreeLook);
                    getLevel().m_cameraController->addHeadRotationXY(
                        -FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.y / 2000), -42_deg, 22_deg,
                        FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.x / 2000), -44_deg, 44_deg
                    );
                    getLevel().m_cameraController->setTorsoRotation(getLevel().m_cameraController->getHeadRotation());

                    return;
                }

                if( getLevel().m_cameraController->getCamOverrideType() == CamOverrideType::FreeLook )
                {
                    getLevel().m_cameraController->setCamOverrideType(CamOverrideType::None);
                }

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
                        create(LaraStateId::WalkForward, getLara())->handleInput(collisionInfo);
                    else
                        create(LaraStateId::RunForward, getLara())->handleInput(collisionInfo);
                }
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                {
                    if( getLevel().m_inputHandler->getInputState().moveSlow )
                        create(LaraStateId::WalkBackward, getLara())->handleInput(collisionInfo);
                    else
                        setTargetState(LaraStateId::RunBack);
                }
            }
        };
    }
}
