#pragma once

#include "abstractstatehandler.h"
#include "engine/audioengine.h"
#include "engine/collisioninfo.h"
#include "engine/objects/laraobject.h"

namespace engine::lara
{
class StateHandler_9 final : public AbstractStateHandler
{
public:
  explicit StateHandler_9(objects::LaraObject& lara)
      : AbstractStateHandler{lara, LaraStateId::FreeFall}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    if(getLara().m_state.fallspeed >= core::DeadlyFallSpeedThreshold)
    {
      getLara().playSoundEffect(TR1SoundEffect::LaraScream);
    }
    dampenHorizontalSpeed(0.05f);
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    auto& laraState = getLara().m_state;
    collisionInfo.floorCollisionRangeMin = core::HeightLimit;
    collisionInfo.floorCollisionRangeMax = -core::ClimbLimit2ClickMin;
    collisionInfo.ceilingCollisionRangeMin = 192_len;
    collisionInfo.facingAngle = getMovementAngle();
    laraState.falling = true;
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);
    jumpAgainstWall(collisionInfo);
    if(collisionInfo.mid.floor.y > 0_len)
    {
      return;
    }

    if(applyLandingDamage())
    {
      setGoalAnimState(LaraStateId::Death);
    }
    else
    {
      setAnimation(AnimationId::LANDING_HARD);
      setGoalAnimState(LaraStateId::Stop);
      setCurrentAnimState(LaraStateId::Stop);
    }
    getWorld().getAudioEngine().stopSoundEffect(TR1SoundEffect::LaraScream, &getLara().m_state);
    getLara().m_state.fallspeed = 0_spd;
    placeOnFloor(collisionInfo);
    getLara().m_state.falling = false;
  }
};
} // namespace engine::lara
