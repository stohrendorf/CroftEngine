#pragma once

#include "abstractstatehandler.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/collisioninfo.h"

#include <gsl/gsl-lite.hpp>

namespace engine::lara
{
class StateHandler_23 final : public AbstractStateHandler
{
public:
  explicit StateHandler_23(const gsl::not_null<objects::LaraObject*>& lara)
      : AbstractStateHandler{lara, LaraStateId::RollBackward}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
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

    if(collisionInfo.mid.floor.dy <= 200_len)
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
