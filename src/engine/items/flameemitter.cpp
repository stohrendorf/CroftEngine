#include "flameemitter.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void FlameEmitter::update()
{
    if( m_state.updateActivationTimeout() )
    {
        if( m_flame != nullptr )
            return;

        m_flame = std::make_shared<FlameParticle>( m_state.position, getLevel() );
        setParent( m_flame, m_state.position.room->node );
        getLevel().m_particles.emplace_back( m_flame );
    }
    else if( m_flame != nullptr )
    {
        removeParticle();
        getLevel().stopSound( TR1SoundId::Burning, m_flame.get() );
    }
}

void FlameEmitter::removeParticle()
{
    if( m_flame == nullptr )
        return;

    auto it = std::find_if( getLevel().m_particles.begin(), getLevel().m_particles.end(),
                            [f = m_flame](const auto& p) {
                                return f == p.get();
                            } );
    if( it != getLevel().m_particles.end() )
        getLevel().m_particles.erase( it );

    setParent( m_flame, nullptr );
    m_flame.reset();
}
}
}
