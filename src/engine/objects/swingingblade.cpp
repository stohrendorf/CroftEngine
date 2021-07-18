#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "engine/particle.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
void SwingingBlade::update()
{
  if(!m_state.updateActivationTimeout())
  {
    if(m_state.current_anim_state == 2_as)
    {
      m_state.goal_anim_state = 0_as;
    }
  }
  else
  {
    if(m_state.current_anim_state == 0_as)
    {
      m_state.goal_anim_state = 2_as;
    }
  }

  if(m_state.current_anim_state == 2_as && m_state.touch_bits.any())
  {
    getWorld().getObjectManager().getLara().m_state.is_hit = true;
    getWorld().getObjectManager().getLara().m_state.health -= 100_hp;

    const core::TRVec splatPos{
      getWorld().getObjectManager().getLara().m_state.position.position.X + util::rand15s(128_len),
      getWorld().getObjectManager().getLara().m_state.position.position.Y - util::rand15(745_len),
      getWorld().getObjectManager().getLara().m_state.position.position.Z + util::rand15s(128_len)};
    auto fx = createBloodSplat(getWorld(),
                               RoomBoundPosition{m_state.position.room, splatPos},
                               getWorld().getObjectManager().getLara().m_state.speed,
                               getWorld().getObjectManager().getLara().m_state.rotation.Y + util::rand15s(+22_deg));
    getWorld().getObjectManager().registerParticle(fx);
  }

  auto room = m_state.position.room;
  const auto sector = findRealFloorSector(m_state.position.position, &room);
  setCurrentRoom(room);
  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects()).y;

  ModelObject::update();
}

void SwingingBlade::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    {
      testBoneCollision(getWorld().getObjectManager().getLara());
    }
  }
  else if(m_state.triggerState != TriggerState::Invisible
          && isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius)
          && testBoneCollision(getWorld().getObjectManager().getLara()))
  {
    if(collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    {
      enemyPush(collisionInfo, false, true);
    }
  }
}
} // namespace engine::objects
