#include "trapdoorup.h"

namespace engine::objects
{
void TrapDoorUp::update()
{
  ModelObject::update();
  auto pos = m_state.position;
  world::findRealFloorSector(pos);
  setCurrentRoom(pos.room);
}
} // namespace engine::objects
