#pragma once

#include "statehandler_onwater.h"
#include "engine/cameracontroller.h"

namespace engine
{
namespace lara
{
class StateHandler_33 final
    : public StateHandler_OnWater
{
public:
    explicit StateHandler_33(LaraNode& lara)
        : StateHandler_OnWater(lara, LaraStateId::OnWaterStop)
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        int spd = std::max(0, getLara().m_state.fallspeed - 4);
        getLara().m_state.fallspeed = spd;

        if( getLara().m_state.health <= 0 )
        {
            setTargetState(LaraStateId::WaterDeath);
            return;
        }

        if( getLevel().m_inputHandler->getInputState().freeLook )
        {
            getLevel().m_cameraController->setMode(CameraMode::FreeLook);
            getLara().addHeadRotationXY(
                -FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.y / 2000), -40_deg, 40_deg,
                FreeLookMouseMovementScale * (getLevel().m_inputHandler->getInputState().mouseMovement.x / 2000), -50_deg, 50_deg
                                       );

            auto torsoRot = getLara().getTorsoRotation();
            torsoRot.X = 0_deg;
            torsoRot.Y = getLara().getHeadRotation().Y / 2;

            getLara().setTorsoRotation(torsoRot);

            return;
        }

        if( getLevel().m_cameraController->getMode() == CameraMode::FreeLook )
        {
            getLevel().m_cameraController->setMode(CameraMode::Chase);
        }

        if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Left )
        {
            getLara().m_state.rotation.Y -= 4_deg;
        }
        else if( getLevel().m_inputHandler->getInputState().xMovement == AxisMovement::Right )
        {
            getLara().m_state.rotation.Y += 4_deg;
        }

        if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Forward )
        {
            setTargetState(LaraStateId::OnWaterForward);
        }
        else if( getLevel().m_inputHandler->getInputState().zMovement == AxisMovement::Backward )
        {
            setTargetState(LaraStateId::OnWaterBackward);
        }

        if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Left )
        {
            setTargetState(LaraStateId::OnWaterLeft);
        }
        else if( getLevel().m_inputHandler->getInputState().stepMovement == AxisMovement::Right )
        {
            setTargetState(LaraStateId::OnWaterRight);
        }

        if( !getLevel().m_inputHandler->getInputState().jump )
        {
            setSwimToDiveKeypressDuration(0);
            return;
        }

        addSwimToDiveKeypressDuration(1);
        if( getSwimToDiveKeypressDuration() < 10 )
        {
            return;
        } // not yet allowed to dive

        setTargetState(LaraStateId::UnderwaterForward);
        setAnimIdGlobal(loader::AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041);
        getLara().m_state.rotation.X = -45_deg;
        getLara().m_state.fallspeed = 80;
        setUnderwaterState(UnderwaterState::Diving);
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle(getLara().m_state.rotation.Y);
        commonOnWaterHandling(collisionInfo);
    }
};
}
}
