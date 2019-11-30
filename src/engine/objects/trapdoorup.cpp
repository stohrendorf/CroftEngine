#include "trapdoorup.h"

namespace engine::objects
{
void TrapDoorUp::update()
{
  ModelObject::update();
  auto pos = m_state.position;
  loader::file::findRealFloorSector(pos);
  setCurrentRoom(pos.room);
}
} // namespace engine
