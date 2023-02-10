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
      setAnimation(AnimationId::ROLL_BEGIN, getWorld().getAnimation(AnimationId::ROLL_BEGIN).firstFrame + 2_frame);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::RollForward);
      return;
    }

    setGoalAnimState(LaraStateId::Stop);

    if(inputHandler.hasAction(hid::Action::FreeLook))
    {
      getWorld().getCameraController().setMode(CameraMode::FreeLook);
      switch(inputHandler.getInputState().xMovement)
      {
      case hid::AxisMovement::Right:
        getLara().addHeadRotationY(core::FreeLookHeadTurnSpeed, -44_deg, 44_deg);
        break;
      case hid::AxisMovement::Left:
        getLara().addHeadRotationY(-core::FreeLookHeadTurnSpeed, -44_deg, 44_deg);
        break;
      case hid::AxisMovement::Null:
        break;
      }

      switch(inputHandler.getInputState().zMovement)
      {
      case hid::AxisMovement::Forward:
        getLara().addHeadRotationX(-core::FreeLookHeadTurnSpeed, -42_deg, 22_deg);
        break;
      case hid::AxisMovement::Backward:
        getLara().addHeadRotationX(core::FreeLookHeadTurnSpeed, -42_deg, 22_deg);
        break;
      case hid::AxisMovement::Null:
        break;
      }

      getLara().setTorsoRotation(getLara().getHeadRotation());

      return;
    }

    if(getWorld().getCameraController().getMode() == CameraMode::FreeLook)
    {
      getWorld().getCameraController().setMode(CameraMode::Chase);
    }

    switch(inputHandler.getInputState().stepMovement)
    {
    case hid::AxisMovement::Right:
      setGoalAnimState(LaraStateId::StepRight);
      break;
    case hid::AxisMovement::Left:
      setGoalAnimState(LaraStateId::StepLeft);
      break;
    case hid::AxisMovement::Null:
      break;
    }

    switch(inputHandler.getInputState().xMovement)
    {
    case hid::AxisMovement::Right:
      setGoalAnimState(LaraStateId::TurnRightSlow);
      break;
    case hid::AxisMovement::Left:
      setGoalAnimState(LaraStateId::TurnLeftSlow);
      break;
    case hid::AxisMovement::Null:
      break;
    }

    if(inputHandler.hasAction(hid::Action::Jump))
    {
      setGoalAnimState(LaraStateId::JumpPrepare);
    }
    else
    {
      switch(inputHandler.getInputState().zMovement)
      {
      case hid::AxisMovement::Forward:
        if(inputHandler.hasAction(hid::Action::Walk))
        {
          create(LaraStateId::WalkForward, getLara())->handleInput(collisionInfo);
        }
        else
        {
          create(LaraStateId::RunForward, getLara())->handleInput(collisionInfo);
        }
        break;
      case hid::AxisMovement::Backward:
        if(inputHandler.hasAction(hid::Action::Walk))
        {
          create(LaraStateId::WalkBackward, getLara())->handleInput(collisionInfo);
        }
        else
        {
          setGoalAnimState(LaraStateId::RunBack);
        }
        break;
      case hid::AxisMovement::Null:
        break;
      }
    }
  }
};
} // namespace engine::lara
