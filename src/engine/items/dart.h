#pragma once

#include "itemnode.h"

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

            // TODO: show blood splatter
        }

        ModelItemNode::update();

        auto room = m_state.position.room;
        auto sector = getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) );
        if( room != m_state.position.room )
            setCurrentRoom( room );

        HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
        m_state.floor = h.distance;

        if( m_state.position.position.Y < m_state.floor )
            return;

        kill();

        // TODO: fx
    }
};
}
}
