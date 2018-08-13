#pragma once

#include "itemnode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
class Dart final : public ModelItemNode
{
public:
    Dart(const gsl::not_null<level::Level*>& level,
         const std::string& name,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
    }

    void update() override
    {
        if( m_state.touch_bits != 0 )
        {
            getLevel().m_lara->m_state.health -= 50;
            getLevel().m_lara->m_state.is_hit = true;

            auto fx = engine::createBloodSplat(
                    getLevel(),
                    m_state.position,
                    m_state.speed,
                    m_state.rotation.Y
            );
            getLevel().m_particles.emplace_back( fx );
        }

        ModelItemNode::update();

        auto room = m_state.position.room;
        auto sector = to_not_null( getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) ) );
        if( room != m_state.position.room )
            setCurrentRoom( room );

        HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
        m_state.floor = h.distance;

        if( m_state.position.position.Y < m_state.floor )
            return;

        kill();

        const auto particle = make_not_null_shared<engine::RicochetParticle>( m_state.position, getLevel() );
        gameplay::setParent( particle, m_state.position.room->node );
        particle->angle = m_state.rotation;
        particle->timePerSpriteFrame = 6;
        getLevel().m_particles.emplace_back( particle );
    }
};
}
}
