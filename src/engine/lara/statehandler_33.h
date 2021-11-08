#pragma once

#include "engine/cameracontroller.h"
#include "statehandler_onwater.h"

namespace engine::lara
{
class StateHandler_33 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_33(objects::LaraObject& lara)
      : StateHandler_OnWater{lara, LaraStateId::OnWaterStop}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    getLara().m_state.fallspeed += -4_spd / 1_frame;
    if(getLara().m_state.fallspeed <= 4_spd)
      getLara().m_state.fallspeed.stop();

    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.hasAction(hid::Action::FreeLook))
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);

      if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
      {
        getLara().addHeadRotationY(-core::FreeLookHeadTurnSpeed, -50_deg, 50_deg);
      }
      else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
      {
        getLara().addHeadRotationY(core::FreeLookHeadTurnSpeed, -50_deg, 50_deg);
      }

      if(inputHandler.getInputState().zMovement == hid::AxisMovement::Forward)
      {
        getLara().addHeadRotationX(-core::FreeLookHeadTurnSpeed, -40_deg, 40_deg);
      }
      else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward)
      {
        getLara().addHeadRotationX(core::FreeLookHeadTurnSpeed, -40_deg, 40_deg);
      }

      auto torsoRot = getLara().getTorsoRotation();
      torsoRot.X = 0_deg;
      torsoRot.Y = getLara().getHeadRotation().Y / 2;

      getLara().setTorsoRotation(torsoRot);

      return;
    }

    if(getWorld().getCameraController().getMode() == CameraMode::FreeLook)
    {
      getWorld().getCameraController().setMode(CameraMode::Chase);
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= core::OnWaterTurnSpeed * 1_rframe;
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += core::OnWaterTurnSpeed * 1_rframe;
    }

    if(inputHandler.getInputState().zMovement == hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::OnWaterForward);
    }
    else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward)
    {
      setGoalAnimState(LaraStateId::OnWaterBackward);
    }

    if(inputHandler.getInputState().stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::OnWaterLeft);
    }
    else if(inputHandler.getInputState().stepMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::OnWaterRight);
    }

    if(!inputHandler.hasAction(hid::Action::Jump))
    {
      setSwimToDiveKeypressDuration(0_rframe);
      return;
    }

    addSwimToDiveKeypressDuration(1_rframe);
    if(getSwimToDiveKeypressDuration() != (core::RenderFrameRate * 1_sec / 3).cast<core::RenderFrame>())
    {
      // not yet allowed to dive; not that the keypress duration is always >10 when coming up from diving
      return;
    }

    setAnimation(AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE);
    setGoalAnimState(LaraStateId::UnderwaterForward);
    setCurrentAnimState(LaraStateId::UnderwaterDiving);
    getLara().m_state.rotation.X = -45_deg;
    getLara().m_state.fallspeed = 80_spd;
    setUnderwaterState(objects::UnderwaterState::Diving);
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    if(doPhysics)
      setMovementAngle(getLara().m_state.rotation.Y);

    commonOnWaterHandling(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
