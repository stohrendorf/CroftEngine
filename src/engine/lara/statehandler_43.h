#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_43 final : public AbstractStateHandler
{
public:
  explicit StateHandler_43(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::InsertPuzzle}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    setCameraRotationAroundLara(-25_deg, -80_deg);
    setCameraDistance(1024_len);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    setMovementAngle(getLara().m_state.rotation.Y);
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);
  }
};
} // namespace engine::lara
