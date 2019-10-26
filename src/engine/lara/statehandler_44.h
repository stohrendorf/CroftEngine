#pragma once

#include "engine/laranode.h"
#include "statehandler_underwater.h"

namespace engine
{
namespace lara
{
class StateHandler_44 final : public StateHandler_Underwater
{
  public:
  explicit StateHandler_44(LaraNode& lara)
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
    getLara().m_state.health = -1_hp;
    setAir(-1_frame);
    setHandStatus(HandStatus::Grabbing);
    auto h = getLara().getWaterSurfaceHeight();
    if(h.is_initialized() && *h < getLara().m_state.position.position.Y - core::DefaultCollisionRadius)
    {
      getLara().m_state.position.position.Y -= 5_len;
    }

    StateHandler_Underwater::postprocessFrame(collisionInfo);
  }
};
} // namespace lara
} // namespace engine
