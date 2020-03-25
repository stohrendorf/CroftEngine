#include "swingingblade.h"

#include "engine/heightinfo.h"
#include "engine/particle.h"
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
    getEngine().getLara().m_state.is_hit = true;
    getEngine().getLara().m_state.health -= 100_hp;

    const core::TRVec splatPos{getEngine().getLara().m_state.position.position.X + util::rand15s(128_len),
                               getEngine().getLara().m_state.position.position.Y - util::rand15(745_len),
                               getEngine().getLara().m_state.position.position.Z + util::rand15s(128_len)};
    auto fx = createBloodSplat(getEngine(),
                               core::RoomBoundPosition{m_state.position.room, splatPos},
                               getEngine().getLara().m_state.speed,
                               getEngine().getLara().m_state.rotation.Y + util::rand15s(+22_deg));
    getEngine().getObjectManager().registerParticle(fx);
  }

  auto room = m_state.position.room;
  const auto sector = findRealFloorSector(m_state.position.position, &room);
  setCurrentRoom(room);
  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getObjectManager().getObjects()).y;

  ModelObject::update();
}

void SwingingBlade::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(isNear(getEngine().getLara(), collisionInfo.collisionRadius))
    {
      testBoneCollision(getEngine().getLara());
    }
  }
  else if(m_state.triggerState != TriggerState::Invisible
          && isNear(getEngine().getLara(), collisionInfo.collisionRadius) && testBoneCollision(getEngine().getLara()))
  {
    if(collisionInfo.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    {
      enemyPush(collisionInfo, false, true);
    }
  }
}
} // namespace engine::objects
