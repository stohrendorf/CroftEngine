#pragma once

#include "engine/cameracontroller.h"
#include "statehandler_onwater.h"

namespace engine::lara
{
class StateHandler_33 final : public StateHandler_OnWater
{
public:
  explicit StateHandler_33(const gsl::not_null<objects::LaraObject*>& lara)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.hasAction(hid::Action::FreeLook))
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);

      switch(inputHandler.getInputState().xMovement)
      {
      case hid::AxisMovement::Left:
        getLara().addHeadRotationY(-core::FreeLookHeadTurnSpeed, -50_deg, 50_deg);
        break;
      case hid::AxisMovement::Right:
        getLara().addHeadRotationY(core::FreeLookHeadTurnSpeed, -50_deg, 50_deg);
        break;
      case hid::AxisMovement::Null:
        break;
      }

      switch(inputHandler.getInputState().zMovement)
      {
      case hid::AxisMovement::Forward:
        getLara().addHeadRotationX(-core::FreeLookHeadTurnSpeed, -40_deg, 40_deg);
        break;
      case hid::AxisMovement::Negative:
        getLara().addHeadRotationX(core::FreeLookHeadTurnSpeed, -40_deg, 40_deg);
        break;
      case hid::AxisMovement::Null:
        break;
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

    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      getLara().m_state.rotation.Y += core::OnWaterTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Left:
      getLara().m_state.rotation.Y -= core::OnWaterTurnSpeed * 1_frame;
      break;
    case hid::AxisMovement::Null:
      break;
    }

    switch(inputHandler.getInputState().zMovement)
    {
    case hid::AxisMovement::Forward:
      setGoalAnimState(LaraStateId::OnWaterForward);
      break;
    case hid::AxisMovement::Backward:
      setGoalAnimState(LaraStateId::OnWaterBackward);
      break;
    case hid::AxisMovement::Null:
      break;
    }

    switch(inputHandler.getInputState().stepMovement)
    {
    case hid::AxisMovement::Right:
      setGoalAnimState(LaraStateId::OnWaterRight);
      break;
    case hid::AxisMovement::Left:
      setGoalAnimState(LaraStateId::OnWaterLeft);
      break;
    case hid::AxisMovement::Null:
      break;
    }

    if(!inputHandler.hasAction(hid::Action::Jump))
    {
      setSwimToDiveKeypressDuration(0_frame);
      return;
    }

    addSwimToDiveKeypressDuration(1_frame);
    if(getSwimToDiveKeypressDuration() != core::FrameRate * 1_sec / 3)
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

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    commonOnWaterHandling(collisionInfo);
  }
};
} // namespace engine::lara
