#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "engine/laranode.h"
#include "level/level.h"

namespace engine
{
namespace items
{
void SwingingBlade::update()
{
    if( !m_state.updateActivationTimeout() )
    {
        if( m_state.current_anim_state == 2 )
        {
            m_state.goal_anim_state = 0;
        }
    }
    else
    {
        if( m_state.current_anim_state == 0 )
        {
            m_state.goal_anim_state = 2;
        }
    }

    if( m_state.current_anim_state == 2 && m_state.touch_bits != 0 )
    {
        getLevel().m_lara->m_state.is_hit = true;
        getLevel().m_lara->m_state.health -= 100;

        core::TRCoordinates splatPos{
                getLevel().m_lara->m_state.position.position.X + util::rand15s() / 256,
                getLevel().m_lara->m_state.position.position.Y - util::rand15() / 44,
                getLevel().m_lara->m_state.position.position.Z + util::rand15s() / 256
        };
        auto fx = engine::createBloodSplat( getLevel(),
                                            core::RoomBoundPosition{m_state.position.room, splatPos},
                                            getLevel().m_lara->m_state.speed,
                                            getLevel().m_lara->m_state.rotation.Y + util::rand15s( +22_deg ) );
        getLevel().m_particles.emplace_back( fx );
    }

    auto room = m_state.position.room;
    auto sector = to_not_null( getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) ) );
    setCurrentRoom( room );
    const int h = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes ).distance;
    m_state.floor = h;

    ModelItemNode::update();
}
}
}
