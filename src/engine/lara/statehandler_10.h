#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
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
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Left
       || getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::ShimmyLeft);
    }
    else if(getWorld().getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Right
            || getWorld().getPresenter().getInputHandler().getInputState().stepMovement == hid::AxisMovement::Right)
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

    const auto frontHeight = collisionInfo.front.floorSpace.y;
    const auto frontSpace = frontHeight - collisionInfo.front.ceilingSpace.y;
    const auto frontLeftSpace = collisionInfo.frontLeft.floorSpace.y - collisionInfo.frontLeft.ceilingSpace.y;
    const auto frontRightSpace = collisionInfo.frontRight.floorSpace.y - collisionInfo.frontRight.ceilingSpace.y;
    if(frontHeight <= -850_len || frontHeight >= -650_len || frontSpace < 0_len || frontLeftSpace < 0_len
       || frontRightSpace < 0_len || collisionInfo.hasStaticMeshCollision)
    {
      return;
    }

    if(getWorld().getPresenter().getInputHandler().hasAction(hid::Action::MoveSlow))
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
