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

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    setCameraRotationAroundLara(-60_deg, 0_deg);
    if(getEngine().getInputHandler().getInputState().xMovement != hid::AxisMovement::Left
       && getEngine().getInputHandler().getInputState().stepMovement != hid::AxisMovement::Left)
    {
      setGoalAnimState(LaraStateId::Hang);
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
    commonEdgeHangHandling(collisionInfo);
    setMovementAngle(getLara().m_state.rotation.Y - 90_deg);
  }
};
} // namespace engine::lara
