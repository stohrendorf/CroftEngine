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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.hasAction(hid::Action::Roll))
    {
      setAnimation(AnimationId::ROLL_BEGIN);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::RollForward);
      return;
    }

    setGoalAnimState(LaraStateId::Stop);

    if(inputHandler.hasAction(hid::Action::FreeLook))
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);
      if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
      {
        getLara().addHeadRotationY(-core::FreeLookHeadTurnSpeed, -44_deg, 44_deg);
      }
      else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
      {
        getLara().addHeadRotationY(core::FreeLookHeadTurnSpeed, -44_deg, 44_deg);
      }

      if(inputHandler.getInputState().zMovement == hid::AxisMovement::Forward)
      {
        getLara().addHeadRotationX(-core::FreeLookHeadTurnSpeed, -42_deg, 22_deg);
      }
      else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward)
      {
        getLara().addHeadRotationX(core::FreeLookHeadTurnSpeed, -42_deg, 22_deg);
      }

      getLara().setTorsoRotation(getLara().getHeadRotation());

      return;
    }

    if(getWorld().getCameraController().getMode() == CameraMode::FreeLook)
    {
      getWorld().getCameraController().setMode(CameraMode::Chase);
    }

    if(inputHandler.getInputState().stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::StepLeft);
    }
    else if(inputHandler.getInputState().stepMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::StepRight);
    }

    if(inputHandler.getInputState().xMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::TurnLeftSlow);
    }
    else if(inputHandler.getInputState().xMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::TurnRightSlow);
    }

    if(inputHandler.hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::JumpPrepare);
    }
    else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Forward)
    {
      if(inputHandler.hasAction(hid::Action::Walk))
      {
        create(LaraStateId::WalkForward, getLara())->handleInput(collisionInfo);
      }
      else
      {
        create(LaraStateId::RunForward, getLara())->handleInput(collisionInfo);
      }
    }
    else if(inputHandler.getInputState().zMovement == hid::AxisMovement::Backward)
    {
      if(inputHandler.hasAction(hid::Action::Walk))
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
