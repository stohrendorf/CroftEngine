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

  void handleInput(CollisionInfo& /*collisionInfo*/, bool /*doPhysics*/) override
  {
    auto& laraState = getLara().m_state;
    laraState.fallspeed += -core::OnWaterAcceleration;
    if(laraState.fallspeed < core::OnWaterMaxSpeed)
      laraState.fallspeed.stop();

    static constexpr auto UprightDelta = toRenderUnit(2_deg / 1_frame) * 1_rframe;
    if(laraState.rotation.X > UprightDelta)
    {
      laraState.rotation.X -= UprightDelta;
    }
    else if(laraState.rotation.X < -UprightDelta)
    {
      laraState.rotation.X += UprightDelta;
    }
    else
    {
      laraState.rotation.X = 0_deg;
    }
  }

  void postprocessFrame(CollisionInfo& collisionInfo, bool doPhysics) override
  {
    getLara().m_state.health = core::DeadHealth;
    setAir(-1_rframe);
    setHandStatus(objects::HandStatus::Grabbing);
    auto h = getLara().getWaterSurfaceHeight();
    if(h.has_value() && *h < getLara().m_state.location.position.Y - core::DefaultCollisionRadius)
    {
      getLara().m_state.location.position.Y -= toRenderUnit(5_spd) * 1_rframe;
    }

    StateHandler_Underwater::postprocessFrame(collisionInfo, doPhysics);
  }
};
} // namespace engine::lara
