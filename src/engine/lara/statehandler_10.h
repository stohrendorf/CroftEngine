#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/cameracontroller.h"
#include "engine/collisioninfo.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputstate.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_10 final : public AbstractStateHandler
{
public:
  explicit StateHandler_10(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::Hang}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    getWorld().getCameraController().setRotationAroundLara(-60_deg, 0_deg);
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    if(const auto& inputState = getWorld().getPresenter().getInputHandler().getInputState();
       inputState.xMovement == hid::AxisMovement::Left || inputState.stepMovement == hid::AxisMovement::Left)
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

    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().zMovement != hid::AxisMovement::Forward)
    {
      return;
    }

    const auto frontHeight = collisionInfo.front.floor.dy;
    const auto frontSpace = frontHeight - collisionInfo.front.ceiling.dy;
    const auto frontLeftSpace = collisionInfo.frontLeft.floor.dy - collisionInfo.frontLeft.ceiling.dy;
    if(const auto frontRightSpace = collisionInfo.frontRight.floor.dy - collisionInfo.frontRight.ceiling.dy;
       frontHeight <= -850_len || frontHeight >= -650_len || frontSpace < 0_len || frontLeftSpace < 0_len
       || frontRightSpace < 0_len || collisionInfo.hasStaticMeshCollision)
    {
      return;
    }

    if(inputHandler.hasAction(hid::Action::Walk))
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