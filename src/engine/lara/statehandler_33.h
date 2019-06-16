#pragma once

#include "engine/cameracontroller.h"
#include "statehandler_onwater.h"

namespace engine
{
namespace lara
{
class StateHandler_33 final : public StateHandler_OnWater
{
public:
    explicit StateHandler_33(LaraNode& lara)
        : StateHandler_OnWater{lara, LaraStateId::OnWaterStop}
    {
    }

    void handleInput(CollisionInfo& /*collisionInfo*/) override
    {
        getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - 4_spd);

        if(getLara().m_state.health <= 0_hp)
        {
            setGoalAnimState(LaraStateId::WaterDeath);
            return;
        }

        if(getEngine().getInputHandler().getInputState().freeLook)
        {
            getEngine().getCameraController().setMode(CameraMode::FreeLook);
            getLara().addHeadRotationXY((-hid::FreeLookMouseMovementScale.retype_as<float>()
                                         * (getEngine().getInputHandler().getInputState().mouseMovement.y / 2000))
                                            .retype_as<core::Angle>(),
                                        -40_deg,
                                        40_deg,
                                        (hid::FreeLookMouseMovementScale.retype_as<float>()
                                         * (getEngine().getInputHandler().getInputState().mouseMovement.x / 2000))
                                            .retype_as<core::Angle>(),
                                        -50_deg,
                                        50_deg);

            auto torsoRot = getLara().getTorsoRotation();
            torsoRot.X = 0_deg;
            torsoRot.Y = getLara().getHeadRotation().Y / core::Angle::type{2};

            getLara().setTorsoRotation(torsoRot);

            return;
        }

        if(getEngine().getCameraController().getMode() == CameraMode::FreeLook)
        {
            getEngine().getCameraController().setMode(CameraMode::Chase);
        }

        if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
        {
            getLara().m_state.rotation.Y -= 4_deg;
        }
        else if(getEngine().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
        {
            getLara().m_state.rotation.Y += 4_deg;
        }

        if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
        {
            setGoalAnimState(LaraStateId::OnWaterForward);
        }
        else if(getEngine().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
        {
            setGoalAnimState(LaraStateId::OnWaterBackward);
        }

        if(getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left)
        {
            setGoalAnimState(LaraStateId::OnWaterLeft);
        }
        else if(getEngine().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right)
        {
            setGoalAnimState(LaraStateId::OnWaterRight);
        }

        if(!getEngine().getInputHandler().getInputState().jump)
        {
            setSwimToDiveKeypressDuration(0_frame);
            return;
        }

        addSwimToDiveKeypressDuration(1_frame);
        if(getSwimToDiveKeypressDuration() != 10_frame)
        {
            // not yet allowed to dive; not that the keypress duration is always >10 when coming up from diving
            return;
        }

        setGoalAnimState(LaraStateId::UnderwaterForward);
        setAnimation(AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041_frame);
        getLara().m_state.rotation.X = -45_deg;
        getLara().m_state.fallspeed = 80_spd;
        setUnderwaterState(UnderwaterState::Diving);
    }

    void postprocessFrame(CollisionInfo& collisionInfo) override
    {
        setMovementAngle(getLara().m_state.rotation.Y);
        commonOnWaterHandling(collisionInfo);
    }
};
} // namespace lara
} // namespace engine
