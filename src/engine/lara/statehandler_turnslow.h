#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_TurnSlow : public AbstractStateHandler
{
protected:
  explicit StateHandler_TurnSlow(const gsl::not_null<objects::LaraObject*>& lara, const LaraStateId id)
      : AbstractStateHandler{lara, id}
  {
  }

public:
  void postprocessFrame(CollisionInfo& collisionInfo) final
  {
    auto& laraState = getLara().m_state;
    collisionInfo.facingAngle = laraState.rotation.Y;
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::ClimbLimit2ClickMin};
    collisionInfo.validCeilingHeightMin = 0_len;
    collisionInfo.policies |= CollisionInfo::SlopeBlockingPolicy;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    setMovementAngle(collisionInfo.facingAngle);
    laraState.fallspeed = 0_spd;
    laraState.falling = false;

    if(collisionInfo.mid.floor.dy <= core::DefaultCollisionRadius)
    {
      if(!tryStartSlide(collisionInfo))
      {
        placeOnFloor(collisionInfo);
      }

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
