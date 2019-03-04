#include "collapsiblefloor.h"

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

    if( m_state.current_anim_state == 0_as ) // stationary
    {
        if( m_state.position.position.Y - 512_len != getEngine().m_lara->m_state.position.position.Y )
        {
            m_state.triggerState = TriggerState::Inactive;
            deactivate();
            return;
        }
        m_state.goal_anim_state = 1_as;
    }
    else if( m_state.current_anim_state == 1_as ) // shaking
    {
        m_state.goal_anim_state = 2_as;
    }
    else if( m_state.current_anim_state == 2_as && m_state.goal_anim_state != 3_as ) // falling, not going to settle
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
    const auto sector = loader::file::findRealFloorSector( m_state.position.position, &room );
    setCurrentRoom( room );

    const HeightInfo h = HeightInfo::fromFloor( sector, m_state.position.position, getEngine().m_itemNodes );
    m_state.floor = h.y;
    if( m_state.current_anim_state != 2_as || m_state.position.position.Y < h.y )
        return;

    // settle
    m_state.goal_anim_state = 3_as;
    m_state.fallspeed = 0_spd;
    auto pos = m_state.position.position;
    pos.Y = m_state.floor;
    m_state.position.position = pos;
    m_state.falling = false;
}
}
}
