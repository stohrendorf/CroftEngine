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
    getLara().m_state.fallspeed = std::max(0_spd, getLara().m_state.fallspeed - 8_spd);

    if(getLara().m_state.rotation.X > 2_deg)
    {
      getLara().m_state.rotation.X -= 2_deg;
    }
    else
    {
      if(getLara().m_state.rotation.X < -2_deg)
      {
        getLara().m_state.rotation.X += 2_deg;
      }
      else
      {
        getLara().m_state.rotation.X = 0_deg;
      }
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo) override
  {
    getLara().m_state.health = core::DeadHealth;
    setAir(-1_frame);
    setHandStatus(objects::HandStatus::Grabbing);
    auto h = getLara().getWaterSurfaceHeight();
    if(h.has_value() && *h < getLara().m_state.position.position.Y - core::DefaultCollisionRadius)
    {
      getLara().m_state.position.position.Y -= 5_len;
    }

    StateHandler_Underwater::postprocessFrame(collisionInfo);
  }
};
} // namespace engine::lara
