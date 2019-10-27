#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine
{
namespace lara
{
class StateHandler_55 final : public AbstractStateHandler
{
public:
  explicit StateHandler_55(LaraNode& lara)
      : AbstractStateHandler{lara, LaraStateId::OnWaterExit}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policyFlags &= ~CollisionInfo::SpazPushPolicy;
    setCameraModifier(CameraModifier::FollowCenter);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);
  }
};
} // namespace lara
} // namespace engine
