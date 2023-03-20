#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_Pushable : public AbstractStateHandler
{
public:
  explicit StateHandler_Pushable(const gsl::not_null<objects::LaraObject*>& lara, const LaraStateId id)
      : AbstractStateHandler{lara, id}
  {
  }

  void handleInput(CollisionInfo& collisionInfo) override
  {
    collisionInfo.policies &= ~CollisionInfo::SpazPushPolicy;
    getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
    getWorld().getCameraController().setRotationAroundLara(-25_deg, 35_deg);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) final
  {
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
  }
};
} // namespace engine::lara
