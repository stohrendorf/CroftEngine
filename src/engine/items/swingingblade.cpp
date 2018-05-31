#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "level/level.h"

namespace engine
{
namespace items
{
void SwingingBlade::update()
{
    if( m_state.updateActivationTimeout() )
    {
        if( m_state.current_anim_state == 0 )
        {
            m_state.goal_anim_state = 2;
        }
    }
    else if( m_state.current_anim_state == 2 )
    {
        m_state.goal_anim_state = 0;
    }

    auto room = m_state.position.room;
    auto sector = getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) );
    setCurrentRoom( room );
    const int h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes,
                                         getLevel().m_floorData )
            .distance;
    m_state.floor = h;

    ModelItemNode::update();
}
}
}
