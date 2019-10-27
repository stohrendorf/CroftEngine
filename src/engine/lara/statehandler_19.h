#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_19 final : public AbstractStateHandler
{
public:
  explicit StateHandler_19(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::Climbing}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);
  }
};
} // namespace lara
} // namespace engine
