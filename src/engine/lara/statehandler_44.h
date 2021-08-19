#pragma once

#include "engine/objects/laraobject.h"
#include "statehandler_underwater.h"

namespace engine::lara
{
class StateHandler_44 final : public StateHandler_Underwater
{
public:
  explicit StateHandler_44(objects::LaraObject& lara)
      : StateHandler_Underwater{lara, LaraStateId::WaterDeath}
  {
  }

  void handleInput(CollisionInfo& /*collisionInfo*/) override
  {
    auto& laraState = getLara().m_state;
    laraState.fallspeed = std::max(0_spd, laraState.fallspeed - 8_spd);

    if(laraState.rotation.X > 2_deg)
    {
      laraState.rotation.X -= 2_deg;
    }
    else
    {
      if(laraState.rotation.X < -2_deg)
      {
        laraState.rotation.X += 2_deg;
      }
      else
      {
        laraState.rotation.X = 0_deg;
      }
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.health = core::DeadHealth;
    setAir(-1_frame);
    setHandStatus(objects::HandStatus::Grabbing);
    auto h = getLara().getWaterSurfaceHeight();
    if(h.has_value() && *h < getLara().m_state.location.position.Y - core::DefaultCollisionRadius)
    {
      getLara().m_state.location.position.Y -= 5_len;
    }

    StateHandler_Underwater::postprocessFrame(collisionInfo);
  }
};
} // namespace engine::lara
