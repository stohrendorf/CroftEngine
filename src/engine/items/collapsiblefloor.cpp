#include "collapsiblefloor.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void CollapsibleFloor::update()
{
    updateLighting();

    if( !m_isActive )
        return;

    if( m_state.current_anim_state == 0 ) // stationary
    {
        if( m_state.position.position.Y - 512 != getLevel().m_lara->m_state.position.position.Y )
        {
            m_state.triggerState = TriggerState::Inactive;
            deactivate();
            return;
        }
        m_state.goal_anim_state = 1;
    }
    else if( m_state.current_anim_state == 1 ) // shaking
    {
        m_state.goal_anim_state = 2;
    }
    else if( m_state.current_anim_state == 2 && m_state.goal_anim_state != 3 ) // falling, not going to settle
    {
        m_state.falling = true;
    }

    ModelItemNode::update();

    if( m_state.triggerState == TriggerState::Deactivated )
    {
        deactivate();
        return;
    }

    auto room = m_state.position.room;
    auto sector = to_not_null( getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) ) );
    setCurrentRoom( room );

    HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
    m_state.floor = h.y;
    if( m_state.current_anim_state != 2 || m_state.position.position.Y < h.y )
        return;

    // settle
    m_state.goal_anim_state = 3;
    m_state.fallspeed = 0;
    auto pos = m_state.position.position;
    pos.Y = m_state.floor;
    m_state.position.position = pos;
    m_state.falling = false;
}
}
}
