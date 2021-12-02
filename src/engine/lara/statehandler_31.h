#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

namespace engine::lara
{
class StateHandler_31 final : public AbstractStateHandler
{
public:
  explicit StateHandler_31(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::ShimmyRight}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    setCameraRotationAroundLara(-60_deg, 0_deg);
    const auto& inputHandler = getWorld().getPresenter().getInputHandler();
    if(inputHandler.getInputState().xMovement != hid::AxisMovement::Right
       && inputHandler.getInputState().stepMovement != hid::AxisMovement::Right)
    {
      setGoalAnimState(LaraStateId::Hang);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
    commonEdgeHangHandling(collisionInfo);
    setMovementAngle(getLara().m_state.rotation.Y + 90_deg);
  }
};
} // namespace engine::lara
