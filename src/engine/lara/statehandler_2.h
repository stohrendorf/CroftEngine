#pragma once

#include "engine/cameracontroller.h"
#include "engine/objects/laraobject.h"
#include "statehandler_standing.h"

namespace engine::lara
{
class StateHandler_2 final : public StateHandler_Standing
{
public:
  explicit StateHandler_2(objects::LaraObject& lara)
      : StateHandler_Standing{lara, LaraStateId::Stop}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    if(getLara().isDead())
    {
      setGoalAnimState(LaraStateId::Death);
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().roll)
    {
      setAnimation(AnimationId::ROLL_BEGIN);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::RollForward);
      return;
    }

    setGoalAnimState(LaraStateId::Stop);

    if(getWorld().getPresenter().getInputHandler().getInputState().freeLook)
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);
      if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
      {
        getLara().addHeadRotationY(-2_deg, -44_deg, 44_deg);
      }
      else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
      {
        getLara().addHeadRotationY(2_deg, -44_deg, 44_deg);
      }

      if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
      {
        getLara().addHeadRotationX(-2_deg, -42_deg, 22_deg);
      }
      else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
      {
        getLara().addHeadRotationX(2_deg, -42_deg, 22_deg);
      }

      getLara().setTorsoRotation(getLara().getHeadRotation());

      return;
    }

    if(getWorld().getCameraController().getMode() == CameraMode::FreeLook)
    {
      getWorld().getCameraController().setMode(CameraMode::Chase);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::StepLeft);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::StepRight);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::TurnLeftSlow);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::TurnRightSlow);
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().jump)
    {
      setGoalAnimState(LaraStateId::JumpPrepare);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Forward)
    {
      if(getWorld().getPresenter().getInputHandler().getInputState().moveSlow)
      {
        create(LaraStateId::WalkForward, getLara())->handleInput(collisionInfo);
      }
      else
      {
        create(LaraStateId::RunForward, getLara())->handleInput(collisionInfo);
      }
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().zMovement == hid::AxisMovement::Backward)
    {
      if(getWorld().getPresenter().getInputHandler().getInputState().moveSlow)
      {
        create(LaraStateId::WalkBackward, getLara())->handleInput(collisionInfo);
      }
      else
      {
        setGoalAnimState(LaraStateId::RunBack);
      }
    }
  }
};
} // namespace engine::lara
