#pragma once

#include "statehandler_onwater.h"
#include "engine/cameracontroller.h"

namespace engine
{
    namespace lara
    {
        class StateHandler_33 final : public StateHandler_OnWater
        {
        public:
            explicit StateHandler_33(LaraNode& lara)
                    : StateHandler_OnWater(lara, LaraStateId::OnWaterStop)
            {
            }


            void handleInput(CollisionInfo& /*collisionInfo*/) override
            {
                if( getHealth() <= 0 )
                {
                    setTargetState(LaraStateId::WaterDeath);
                    return;
                }

                if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
                    setTargetState(LaraStateId::OnWaterForward);
                else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
                    setTargetState(LaraStateId::OnWaterBackward);

                if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
                    setTargetState(LaraStateId::OnWaterLeft);
                else if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
                    setTargetState(LaraStateId::OnWaterRight);

                if( !getLevel().m_inputHandler->getInputState().jump )
                {
                    setSwimToDiveKeypressDuration(0);
                    return;
                }

                //! @todo Check condition
                if(!getSwimToDiveKeypressDuration())
                    return; // not allowed to dive at all

                if(getSwimToDiveKeypressDuration() < 10)
                    return; // not yet allowed to dive

                setTargetState(LaraStateId::UnderwaterForward);
                setAnimIdGlobal(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
                setXRotation(-45_deg);
                setFallSpeed(80);
                setUnderwaterState(UnderwaterState::Diving);
            }

            void animateImpl(CollisionInfo& /*collisionInfo*/) override
            {
                if(getLevel().m_inputHandler->getInputState().freeLook)
                {
                    getLevel().m_cameraController->setCamOverrideType(CamOverrideType::FreeLook);
                    getLevel().m_cameraController->addHeadRotationXY(
                            -FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.y/2000),
                            FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.x/2000)
                    );

                    getLevel().m_cameraController->setTorsoRotation(getLevel().m_cameraController->getHeadRotation());
                }
                else if(getLevel().m_cameraController->getCamOverrideType() == CamOverrideType::FreeLook)
                {
                    getLevel().m_cameraController->setCamOverrideType(CamOverrideType::None);
                }

                setFallSpeed(std::max(0, getFallSpeed() - 4));

                if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
                    m_yRotationSpeed = -4_deg;
                else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
                    m_yRotationSpeed = 4_deg;
                else
                    m_yRotationSpeed = 0_deg;

                addSwimToDiveKeypressDuration(1);
            }


            void postprocessFrame(CollisionInfo& collisionInfo) override
            {
                setMovementAngle(getRotation().Y);
                commonOnWaterHandling(collisionInfo);
            }
        };
    }
}
