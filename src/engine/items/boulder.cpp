#include "boulder.h"

#include "level/level.h"
#include "engine/laranode.h"

void engine::items::RollingBall::update()
{
    if( m_state.triggerState == TriggerState::Active )
    {
        if( m_state.position.position.Y >= m_state.floor )
        {
            if( m_state.current_anim_state == 0 )
            {
                m_state.goal_anim_state = 1;
            }
        }
        else
        {
            if( !m_state.falling )
            {
                m_state.fallspeed = -10;
                m_state.falling = true;
            }
        }
        const auto oldPos = m_state.position.position;
        ModelItemNode::update();

        auto room = m_state.position.room;
        auto sector = getLevel().findRealFloorSector( m_state.position.position, to_not_null( &room ) );
        setCurrentRoom( room );
        const auto hi = HeightInfo::fromFloor( sector, m_state.position.position, getLevel().m_itemNodes );
        m_state.floor = hi.distance;
        getLevel().m_lara->handleCommandSequence( hi.lastCommandSequenceOrDeath, true );
        if( m_state.floor - loader::QuarterSectorSize <= m_state.position.position.Y )
        {
            m_state.fallspeed = 0;
            m_state.falling = false;
            m_state.position.position.Y = m_state.floor;
        }

        // let's see if we hit a wall, and if that's the case, stop.
        const auto testPos = m_state.position.position
                             + core::TRCoordinates( m_state.rotation.Y.sin() * loader::SectorSize / 2,
                                                    0,
                                                    m_state.rotation.Y.cos() * loader::SectorSize / 2 );
        sector = getLevel().findRealFloorSector( testPos, room );
        if( HeightInfo::fromFloor( sector, testPos, getLevel().m_itemNodes ).distance < m_state.position.position.Y )
        {
            m_state.fallspeed = 0;
            m_state.touch_bits = 0;
            m_state.speed = 0;
            m_state.triggerState = TriggerState::Deactivated;
            m_state.position.position.X = oldPos.X;
            m_state.position.position.Y = m_state.floor;
            m_state.position.position.Z = oldPos.Z;
        }
    }
    else if( m_state.triggerState == TriggerState::Deactivated && !m_state.updateActivationTimeout() )
    {
        m_state.triggerState = TriggerState::Deactivated;
        m_state.position.position = m_position.position;
        setCurrentRoom( m_position.room );
        getSkeleton()->setAnimIdGlobal( m_state, getLevel().m_animatedModels[m_state.object_number]->anim_index, 0 );
        m_state.goal_anim_state = m_state.current_anim_state;
        m_state.required_anim_state = 0;
        deactivate();
    }
}
