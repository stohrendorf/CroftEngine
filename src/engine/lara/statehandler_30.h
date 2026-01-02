#pragma once

#include "abstractstatehandler.h"
#include "core/units.h"
#include "engine/collisioninfo.h"
#include "hid/inputstate.h"

#include <gsl-lite/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_30 final : public AbstractStateHandler
{
public:
  explicit StateHandler_30(const gsl_lite::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::ShimmyLeft}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    getWorld().getCameraController().setRotationAroundLara(-60_deg, 0_deg);
    if(const auto& inputHandler = getWorld().getPresenter().getInputHandler();
       inputHandler.getInputState().xMovement != hid::AxisMovement::Left
       && inputHandler.getInputState().stepMovement != hid::AxisMovement::Left)
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