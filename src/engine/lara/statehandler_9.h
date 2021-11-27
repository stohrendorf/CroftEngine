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
    collisionInfo.validFloorHeight = {-core::ClimbLimit2ClickMin, core::HeightLimit};
    collisionInfo.validCeilingHeightMin = 192_len;
    collisionInfo.facingAngle = getMovementAngle();
    collisionInfo.initHeightInfo(laraState.location.position, getWorld(), core::LaraWalkHeight);

    laraState.falling = true;

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
    getLara().m_state.falling = false;
    placeOnFloor(collisionInfo);
  }
};
} // namespace engine::lara
