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

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.falling = false;
    getLara().m_state.fallspeed = 0_spd;
    collisionInfo.facingAngle = getLara().m_state.rotation.Y + 180_deg;
    setMovementAngle(collisionInfo.facingAngle);
    collisionInfo.policies.set(CollisionInfo::PolicyFlags::SlopesAreWalls);
    collisionInfo.badPositiveDistance = core::HeightLimit;
    collisionInfo.badNegativeDistance = -core::ClimbLimit2ClickMin;
    collisionInfo.badCeilingDistance = 0_len;
    collisionInfo.initHeightInfo(getLara().m_state.position.position, getWorld(), core::LaraWalkHeight);

    if(stopIfCeilingBlocked(collisionInfo))
    {
      return;
    }
    if(tryStartSlide(collisionInfo))
    {
      return;
    }

    if(collisionInfo.mid.floorSpace.y <= 200_len)
    {
      applyShift(collisionInfo);
      placeOnFloor(collisionInfo);
      return;
    }

    setAnimation(AnimationId::FREE_FALL_BACK, 1473_frame);
    setGoalAnimState(LaraStateId::FallBackward);
    setCurrentAnimState(LaraStateId::FallBackward);
    getLara().m_state.fallspeed = 0_spd;
    getLara().m_state.falling = true;
  }
};
} // namespace engine::lara
