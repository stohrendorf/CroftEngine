#include "animating.h"

#include "engine/world/room.h"

namespace engine::objects
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

  ModelObject::update();
  auto room = m_state.position.room;
  findRealFloorSector(m_state.position.position, &room);
  setCurrentRoom(room);
}
} // namespace engine::objects
