#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_30 final : public AbstractStateHandler
{
public:
  explicit StateHandler_30(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::ShimmyLeft}
  {
  }

  void handleInput(CollisionInfo& collisionInfo, bool /*doPhysics*/) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    getWorld().getCameraController().setRotationAroundLara(-60_deg, 0_deg);
    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().xMovement != hid::AxisMovement::Left
       && inputHandler.getInputState().stepMovement != hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::Hang);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
    commonEdgeHangHandling(collisionInfo, doPhysics);
    setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
  }
};
} // namespace engine::lara
