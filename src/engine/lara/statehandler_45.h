#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_45 final : public AbstractStateHandler
{
public:
  explicit StateHandler_45(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::RollForward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    auto& laraState = getLara().m_state;
    laraState.falling = false;
    laraState.fallspeed = 0_spd;
    collisionInfo.facingAngle = laraState.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.policies |= CollisionInfo::PolicyFlags::SlopesAreWalls;
    collisionInfo.floorCollisionRangeMin = core::HeightLimit;
    collisionInfo.floorCollisionRangeMax = -core::ClimbLimit2ClickMin;
    collisionInfo.ceilingCollisionRangeMin = 0_len;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

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

    setAnimation(AnimationId::FREE_FALL_FORWARD);
    setGoalAnimState(LaraStateId::JumpForward);
    setCurrentAnimState(LaraStateId::JumpForward);
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = true;
  }
};
} // namespace engine::lara
