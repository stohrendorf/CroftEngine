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
  void postprocessFrame(CollisionInfo& collisionInfo) override final
  {
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = false;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badPositiveDistance = core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.policyFlags |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getEngine(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floorSpace.y <= core::DefaultCollisionRadius)
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
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = true;
  }
};
} // namespace lara
