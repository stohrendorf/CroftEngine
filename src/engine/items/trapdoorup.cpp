#include "trapdoorup.h"

#include "level/level.h"

namespace engine
{
namespace items
{
void TrapDoorUp::update()
{
    ModelItemNode::update();
    auto pos = m_state.position;
    getLevel().findRealFloorSector( pos );
    setCurrentRoom( pos.room );
}
}
}
