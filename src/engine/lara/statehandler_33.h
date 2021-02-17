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

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - 4_spd);

    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::WaterDeath);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::FreeLook))
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);

      if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
      {
        getLara().addHeadRotationY(-2_deg, -50_deg, 50_deg);
      }
      else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
      {
        getLara().addHeadRotationY(2_deg, -50_deg, 50_deg);
      }

      if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
      {
        getLara().addHeadRotationX(-2_deg, -40_deg, 40_deg);
      }
      else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
      {
        getLara().addHeadRotationX(2_deg, -40_deg, 40_deg);
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

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      getLara().m_state.rotation.Y -= 4_deg;
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      getLara().m_state.rotation.Y += 4_deg;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      setGoalAnimState(LaraStateId::OnWaterForward);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
    {
      setGoalAnimState(LaraStateId::OnWaterBackward);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::OnWaterLeft);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::OnWaterRight);
    }

    if(!getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Jump))
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

    setAnimation(AnimationId::FREE_FALL_TO_UNDERWATER_ALTERNATE, 2041_frame);
    setGoalAnimState(LaraStateId::UnderwaterForward);
    setCurrentAnimState(LaraStateId::UnderwaterDiving);
    getLara().m_state.rotation.X = -45_deg;
    getLara().m_state.fallspeed = 80_spd;
    setUnderwaterState(objects::UnderwaterState::Diving);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
