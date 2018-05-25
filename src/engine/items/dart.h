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
        m_state.is_hit = true;
    }

    void update() override
    {
        // TODO: check bone collisions

        ModelItemNode::update();

        const loader::Room* room = m_state.position.room;
        auto sector = getLevel().findRealFloorSector( m_state.position.position, &room );
        if( room != m_state.position.room )
            setCurrentRoom( room );

        HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes,
                                              getLevel().m_floorData );
        m_state.floor = h.distance;

        if( m_state.position.position.Y < m_state.floor )
            return;

        getLevel().scheduleDeletion( this );
        deactivate();

        // TODO: fx
    }
};
}
}
