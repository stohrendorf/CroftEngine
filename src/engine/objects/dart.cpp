#include "dart.h"

#include "engine/particle.h"
#include "engine/world.h"
#include "laraobject.h"

namespace engine::objects
{
void Dart::collide(CollisionInfo& info)
{
  if(!isNear(getWorld().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}

void Dart::update()
{
  if(m_state.touch_bits != 0)
  {
    getWorld().getObjectManager().getLara().m_state.health -= 50_hp;
    getWorld().getObjectManager().getLara().m_state.is_hit = true;

    auto fx = createBloodSplat(getWorld(), m_state.position, m_state.speed, m_state.rotation.Y);
    getWorld().getObjectManager().registerParticle(fx);
  }

  ModelObject::update();

  auto room = m_state.position.room;
  const auto sector = findRealFloorSector(m_state.position.position, &room);
  if(room != m_state.position.room)
    setCurrentRoom(room);

  const HeightInfo h
    = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects());
  m_state.floor = h.y;

  if(m_state.position.position.Y < m_state.floor)
    return;

  kill();

  const auto particle = std::make_shared<RicochetParticle>(m_state.position, getWorld());
  setParent(particle, m_state.position.room->node);
  particle->angle = m_state.rotation;
  particle->timePerSpriteFrame = 6;
  getWorld().getObjectManager().registerParticle(particle);
}
} // namespace engine::objects
