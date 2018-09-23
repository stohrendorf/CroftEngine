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
    if( std::abs( d.X ) > 20 * loader::SectorSize
        || std::abs( d.Y ) > 20 * loader::SectorSize
        || std::abs( d.Z ) > 20 * loader::SectorSize )
        return;

    auto particle = make_not_null_shared<engine::SplashParticle>( m_state.position, getLevel(), true );
    gameplay::setParent( particle, m_state.position.room->node );
    getLevel().m_particles.emplace_back( particle );
}
}
}
