#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_23 final : public AbstractStateHandler
{
public:
  explicit StateHandler_23(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::RollBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool /*doPhysics*/) override
  {
    auto& laraState = getLara().m_state;
    collisionInfo.facingAngle = laraState.rotation.Y + 180_deg;
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::SlopesAreWalls);
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    laraState.fallspeed = 0_spd;
    laraState.falling = false;

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }
    if(tryStartSlide(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floor.y <= 200_len)
    {
      applyShift(collisionInfo);
      placeOnFloor(collisionInfo);
      return;
    }

    setAnimation(AnimationId::FREE_FALL_BACK);
    setGoalAnimState(LaraStateId::FallBackward);
    setCurrentAnimState(LaraStateId::FallBackward);
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = true;
  }
};
} // namespace engine::lara
