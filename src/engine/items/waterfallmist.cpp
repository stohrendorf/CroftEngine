#include "waterfallmist.h"

#include "engine/laranode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
void WaterfallMist::update()
{
    if(!m_state.activationState.isFullyActivated())
        return;

    const auto d = m_state.position.position - getEngine().getLara().m_state.position.position;
    if(abs(d.X) > 20 * core::SectorSize || abs(d.Y) > 20 * core::SectorSize || abs(d.Z) > 20 * core::SectorSize)
        return;

    auto particle = std::make_shared<SplashParticle>(m_state.position, getEngine(), true);
    setParent(particle, m_state.position.room->node);
    getEngine().getParticles().emplace_back(particle);
}
} // namespace items
} // namespace engine
