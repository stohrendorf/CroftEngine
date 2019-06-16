#include "animating.h"

namespace engine
{
namespace items
{
void Animating::update()
{
    if(m_state.updateActivationTimeout())
    {
        m_state.goal_anim_state = 1_as;
    }
    else
    {
        m_state.goal_anim_state = 0_as;
    }

    ModelItemNode::update();
    auto room = m_state.position.room;
    loader::file::findRealFloorSector(m_state.position.position, &room);
    if(room != m_state.position.room)
    {
        setCurrentRoom(room);
    }
}
} // namespace items
}