#include "tallblock.h"

#include "loader/file/level/level.h"

namespace engine
{
namespace items
{
void TallBlock::update()
{
    if( m_state.updateActivationTimeout() )
    {
        if( m_state.current_anim_state == 0_as )
        {
            loader::file::Room::patchHeightsForBlock( *this, 2 * core::SectorSize );
            m_state.goal_anim_state = 1_as;
        }
    }
    else
    {
        if( m_state.current_anim_state == 1_as )
        {
            loader::file::Room::patchHeightsForBlock( *this, 2 * core::SectorSize );
            m_state.goal_anim_state = 0_as;
        }
    }

    ModelItemNode::update();
    auto room = m_state.position.room;
    loader::file::level::Level::findRealFloorSector( m_state.position.position, &room );
    setCurrentRoom( room );

    if( m_state.triggerState != TriggerState::Deactivated )
    {
        return;
    }

    m_state.triggerState = TriggerState::Active;
    loader::file::Room::patchHeightsForBlock( *this, -2 * core::SectorSize );
    auto pos = m_state.position.position;
    pos.X = (pos.X / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
    pos.Z = (pos.Z / core::SectorSize) * core::SectorSize + core::SectorSize / 2;
    m_state.position.position = pos;
}

void TallBlock::load(const YAML::Node& n)
{
    loader::file::Room::patchHeightsForBlock( *this, -2 * core::SectorSize );
    ModelItemNode::load( n );
    loader::file::Room::patchHeightsForBlock( *this, 2 * core::SectorSize );
}
}
}
