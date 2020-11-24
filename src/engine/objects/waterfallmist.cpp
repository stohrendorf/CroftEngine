#include "waterfallmist.h"

#include "engine/particle.h"
#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void WaterfallMist::update()
{
  if(!m_state.activationState.isFullyActivated())
    return;

  const auto d = m_state.position.position - getWorld().getObjectManager().getLara().m_state.position.position;
  if(abs(d.X) > 20 * core::SectorSize || abs(d.Y) > 20 * core::SectorSize || abs(d.Z) > 20 * core::SectorSize)
    return;

  auto particle = std::make_shared<SplashParticle>(m_state.position, getWorld(), true);
  setParent(particle, m_state.position.room->node);
  getWorld().getObjectManager().registerParticle(particle);
}
} // namespace engine::objects
