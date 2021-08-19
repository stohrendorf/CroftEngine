#pragma once

#include "abstractstatehandler.h"
#include "engine/collisioninfo.h"

namespace engine::lara
{
class StateHandler_Standing : public AbstractStateHandler
{
protected:
  explicit StateHandler_Standing(objects::LaraObject& lara, const LaraStateId id)
      : AbstractStateHandler{lara, id}
  {
  }

public:
  void postprocessFrame(CollisionInfo& collisionInfo) final
  {
    auto& laraState = getLara().m_state;
    laraState.fallspeed = 0_spd;
    laraState.falling = false;
    collisionInfo.facingAngle = laraState.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.floorCollisionRangeMax = -core::ClimbLimit2ClickMin;
    collisionInfo.floorCollisionRangeMin = core::ClimbLimit2ClickMin;
    collisionInfo.ceilingCollisionRangeMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floor.y <= core::DefaultCollisionRadius)
    {
      if(!tryStartSlide(collisionInfo))
      {
        applyShift(collisionInfo);
        placeOnFloor(collisionInfo);
      }
      return;
    }

    setAnimation(AnimationId::FREE_FALL_FORWARD, 492_frame);
    setGoalAnimState(LaraStateId::JumpForward);
    setCurrentAnimState(LaraStateId::JumpForward);
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = true;
  }
};
} // namespace engine::lara
