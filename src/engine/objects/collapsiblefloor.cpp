#include "collapsiblefloor.h"

#include "engine/heightinfo.h"
#include "engine/objectmanager.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"

namespace engine::objects
{
void CollapsibleFloor::update()
{
  if(!m_isActive)
    return;

  if(m_state.current_anim_state == 0_as) // stationary
  {
    if(m_state.location.position.Y - 512_len != getWorld().getObjectManager().getLara().m_state.location.position.Y)
    {
      m_state.triggerState = TriggerState::Inactive;
      deactivate();
      return;
    }
    m_state.goal_anim_state = 1_as;
  }
  else if(m_state.current_anim_state == 1_as) // shaking
  {
    m_state.goal_anim_state = 2_as;
  }
  else if(m_state.current_anim_state == 2_as && m_state.goal_anim_state != 3_as) // falling, not going to settle
  {
    m_state.falling = true;
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
    return;
  }

  const auto sector = m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  const HeightInfo h
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
  m_state.floor = h.y;
  if(m_state.current_anim_state != 2_as || m_state.location.position.Y < h.y)
    return;

  // settle
  m_state.goal_anim_state = 3_as;
  m_state.fallspeed = 0_spd;
  m_state.falling = false;
  auto pos = m_state.location.position;
  pos.Y = m_state.floor;
  m_state.location.position = pos;
}
} // namespace engine::objects
