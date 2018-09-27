#include "dart.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
void Dart::collide(LaraNode& lara, CollisionInfo& info)
{
    if( !isNear( lara, info.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !(info.policyFlags & CollisionInfo::EnableBaddiePush) )
        return;

    enemyPush( lara, info, false, true );
}

void Dart::update()
{
    if( m_state.touch_bits != 0 )
    {
        getLevel().m_lara->m_state.health -= 50;
        getLevel().m_lara->m_state.is_hit = true;

        auto fx = createBloodSplat(
                getLevel(),
                m_state.position,
                m_state.speed,
                m_state.rotation.Y
        );
        getLevel().m_particles.emplace_back( fx );
    }

    ModelItemNode::update();

    auto room = m_state.position.room;
    const auto sector = gsl::make_not_null(
            level::Level::findRealFloorSector( m_state.position.position, make_not_null( &room ) ) );
    if( room != m_state.position.room )
        setCurrentRoom( room );

    const HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
    m_state.floor = h.y;

    if( m_state.position.position.Y < m_state.floor )
        return;

    kill();

    const auto particle = make_not_null_shared<RicochetParticle>( m_state.position, getLevel() );
    setParent( particle, m_state.position.room->node );
    particle->angle = m_state.rotation;
    particle->timePerSpriteFrame = 6;
    getLevel().m_particles.emplace_back( particle );
}
}
}
