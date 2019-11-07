#include "barricade.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
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

  ModelItemNode::update();
  auto room = m_state.position.room;
  loader::file::findRealFloorSector(m_state.position.position, &room);
  if(room != m_state.position.room)
  {
    setCurrentRoom(room);
  }
}

void Barricade::collide(CollisionInfo& collisionInfo)
{
  if(!isNear(getEngine().getLara(), collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(getEngine().getLara()))
    return;

  if(!collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(collisionInfo, false, true);
}
} // namespace items
} // namespace engine
