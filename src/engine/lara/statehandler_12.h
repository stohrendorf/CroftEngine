#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_12 final : public AbstractStateHandler
{
public:
  explicit StateHandler_12(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::Unknown12}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);
    applyShift(collisionInfo);
  }
};
} // namespace lara
} // namespace engine
