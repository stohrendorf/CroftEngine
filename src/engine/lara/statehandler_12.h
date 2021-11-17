#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_12 final : public AbstractStateHandler
{
public:
  explicit StateHandler_12(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::Unknown12}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.location.position, getWorld(), core::LaraWalkHeight);
    applyShift(collisionInfo);
  }
};
} // namespace engine::lara
