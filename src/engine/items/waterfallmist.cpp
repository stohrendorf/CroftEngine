#include "waterfallmist.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void WaterfallMist::update()
{
    if( !m_state.activationState.isFullyActivated() )
        return;

    const auto d = m_state.position.position - getLevel().m_lara->m_state.position.position;
    if( abs( d.X ) > 20 * core::SectorSize
        || abs( d.Y ) > 20 * core::SectorSize
        || abs( d.Z ) > 20 * core::SectorSize )
        return;

    auto particle = std::make_shared<SplashParticle>( m_state.position, getLevel(), true );
    setParent( particle, m_state.position.room->node );
    getLevel().m_particles.emplace_back( particle );
}
}
}
