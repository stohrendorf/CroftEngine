#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "engine/world/world.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_10 final : public AbstractStateHandler
{
public:
  explicit StateHandler_10(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::Hang}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    setCameraRotationAroundLara(-60_deg, 0_deg);
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    const auto& inputState = getWorld().getPresenter().getInputHandler().getInputState();
    if(inputState.xMovement == hid::AxisMovement::Left || inputState.stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::ShimmyLeft);
    }
    else if(inputState.xMovement == hid::AxisMovement::Right || inputState.stepMovement == hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::ShimmyRight);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    commonEdgeHangHandling(collisionInfo);

    if(getGoalAnimState() != LaraStateId::Hang)
    {
      return;
    }

    if(getWorld().getPresenter().getInputHandler().getInputState().zMovement != hid::AxisMovement::Forward)
    {
      return;
    }

    const auto frontHeight = collisionInfo.front.floor.y;
    const auto frontSpace = frontHeight - collisionInfo.front.ceiling.y;
    const auto frontLeftSpace = collisionInfo.frontLeft.floor.y - collisionInfo.frontLeft.ceiling.y;
    const auto frontRightSpace = collisionInfo.frontRight.floor.y - collisionInfo.frontRight.ceiling.y;
    if(frontHeight <= -850_len || frontHeight >= -650_len || frontSpace < 0_len || frontLeftSpace < 0_len
       || frontRightSpace < 0_len || collisionInfo.hasStaticMeshCollision)
    {
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Walk))
    {
      setGoalAnimState(LaraStateId::Handstand);
    }
    else
    {
      setGoalAnimState(LaraStateId::Climbing);
    }
  }
};
} // namespace engine::lara
