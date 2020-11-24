#include "barricade.h"

#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Barricade::update()
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

void Barricade::collide(CollisionInfo& collisionInfo)
{
  if(!isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(collisionInfo, false, true);
}
} // namespace engine::objects
