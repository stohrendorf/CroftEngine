#include "flameemitter.h"

#include "engine/laranode.h"
#include "engine/particle.h"

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

        m_flame = std::make_shared<FlameParticle>( m_state.position, getEngine() );
        setParent( m_flame, m_state.position.room->node );
        getEngine().m_particles.emplace_back( m_flame );
    }
    else if( m_flame != nullptr )
    {
        removeParticle();
        getEngine().stopSound( TR1SoundId::Burning, m_flame.get() );
    }
}

void FlameEmitter::removeParticle()
{
    if( m_flame == nullptr )
        return;

    auto it = std::find_if( getEngine().m_particles.begin(), getEngine().m_particles.end(),
                            [f = m_flame](const auto& p) {
                                return f == p.get();
                            } );
    if( it != getEngine().m_particles.end() )
        getEngine().m_particles.erase( it );

    setParent( m_flame, nullptr );
    m_flame.reset();
}
}
}
