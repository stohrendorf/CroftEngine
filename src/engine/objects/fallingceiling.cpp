#include "fallingceiling.h"

#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/quantity.h"

#include <bitset>

namespace engine::objects
{
void FallingCeiling::update()
{
  if(m_state.current_anim_state == 0_as)
  {
    m_state.goal_anim_state = 1_as;
    m_state.falling = true;
  }
  else if(m_state.current_anim_state == 1_as && m_state.touch_bits != 0)
  {
    getWorld().getObjectManager().getLara().m_state.is_hit = true;
    getWorld().getObjectManager().getLara().m_state.health -= 300_hp;
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
  }
  else if(m_state.current_anim_state == 1_as && m_state.location.position.Y >= m_state.floor)
  {
    m_state.goal_anim_state = 2_as;
    m_state.location.position.Y = m_state.floor;
    m_state.fallspeed = 0_spd;
    m_state.falling = false;
  }
}

void FallingCeiling::collide(CollisionInfo& collisionInfo)
{
  trapCollideWithLara(collisionInfo);
}
} // namespace engine::objects
