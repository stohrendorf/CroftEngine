#include "waterfallmist.h"

#include "engine/particle.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void WaterfallMist::update()
{
  if(!m_state.activationState.isFullyActivated())
    return;

  const auto d = m_state.location.position - getWorld().getObjectManager().getLara().m_state.location.position;
  if(abs(d.X) > 20 * core::SectorSize || abs(d.Y) > 20 * core::SectorSize || abs(d.Z) > 20 * core::SectorSize)
    return;

  auto particle = std::make_shared<SplashParticle>(m_state.location, getWorld(), true);
  setParent(particle, m_state.location.room->node);
  getWorld().getObjectManager().registerParticle(particle);
}
} // namespace engine::objects
