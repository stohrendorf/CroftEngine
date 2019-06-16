#include "trapdoorup.h"

namespace engine
{
namespace items
{
void TrapDoorUp::update()
{
    ModelItemNode::update();
    auto pos = m_state.position;
    loader::file::findRealFloorSector(pos);
    setCurrentRoom(pos.room);
}
} // namespace items
} // namespace engine
