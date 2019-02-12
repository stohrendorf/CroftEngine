#include "trapdoorup.h"

#include "loader/file/level/level.h"

namespace engine
{
namespace items
{
void TrapDoorUp::update()
{
    ModelItemNode::update();
    auto pos = m_state.position;
    loader::file::level::Level::findRealFloorSector( pos );
    setCurrentRoom( pos.room );
}
}
}
