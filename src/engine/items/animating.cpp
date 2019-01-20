#include "animating.h"

#include "level/level.h"

namespace engine
{
namespace items
{
void Animating::update()
{
    if( m_state.updateActivationTimeout() )
    {
        m_state.goal_anim_state = 1;
    }
    else
    {
        m_state.goal_anim_state = 0;
    }

    ModelItemNode::update();
    auto room = m_state.position.room;
    level::Level::findRealFloorSector( m_state.position.position, &room );
    if( room != m_state.position.room )
    {
        setCurrentRoom( room );
    }
}
}
}