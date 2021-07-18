#include "boulder.h"

#include "engine/particle.h"
#include "engine/world/world.h"
#include "laraobject.h"

void engine::objects::RollingBall::update()
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(m_state.position.position.Y >= m_state.floor)
    {
      if(m_state.current_anim_state == 0_as)
      {
        m_state.goal_anim_state = 1_as;
      }
    }
    else
    {
      if(!m_state.falling)
      {
        m_state.fallspeed = -10_spd;
        m_state.falling = true;
      }
    }
    const auto oldPos = m_state.position.position;
    ModelObject::update();

    auto room = m_state.position.room;
    auto sector = findRealFloorSector(m_state.position.position, &room);
    setCurrentRoom(room);
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects());
    m_state.floor = hi.y;
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor - core::QuarterSectorSize <= m_state.position.position.Y)
    {
      m_state.fallspeed = 0_spd;
      m_state.falling = false;
      m_state.position.position.Y = m_state.floor;
    }

    // let's see if we hit a wall, and if that's the case, stop.
    const auto testPos = m_state.position.position + util::pitch(core::SectorSize / 2, m_state.rotation.Y);
    sector = findRealFloorSector(testPos, room);
    if(HeightInfo::fromFloor(sector, testPos, getWorld().getObjectManager().getObjects()).y
       < m_state.position.position.Y)
    {
      m_state.fallspeed = 0_spd;
      m_state.touch_bits.reset();
      m_state.speed = 0_spd;
      m_state.triggerState = TriggerState::Deactivated;
      m_state.position.position.X = oldPos.X;
      m_state.position.position.Y = m_state.floor;
      m_state.position.position.Z = oldPos.Z;
    }
  }
  else if(m_state.triggerState == TriggerState::Deactivated && !m_state.updateActivationTimeout())
  {
    m_state.triggerState = TriggerState::Deactivated;
    m_state.position.position = m_position.position;
    setCurrentRoom(m_position.room);
    getSkeleton()->setAnimation(
      m_state.current_anim_state, getWorld().findAnimatedModelForType(m_state.type)->animations, 0_frame);
    m_state.goal_anim_state = m_state.current_anim_state;
    m_state.required_anim_state = 0_as;
    deactivate();
  }
}

void engine::objects::RollingBall::collide(CollisionInfo& collisionInfo)
{
  if(m_state.triggerState != TriggerState::Active)
  {
    if(m_state.triggerState != TriggerState::Invisible)
    {
      if(!isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
        return;

      if(!testBoneCollision(getWorld().getObjectManager().getLara()))
        return;

      if(!collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
        return;

      enemyPush(collisionInfo, false, true);
    }
    return;
  }

  if(!isNear(getWorld().getObjectManager().getLara(), collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(getWorld().getObjectManager().getLara()))
    return;

  if(!getWorld().getObjectManager().getLara().m_state.falling)
  {
    getWorld().getObjectManager().getLara().m_state.is_hit = true;
    if(getWorld().getObjectManager().getLara().isDead())
      return;

    getWorld().getObjectManager().getLara().m_state.health = core::DeadHealth;
    getWorld().getObjectManager().getLara().setCurrentRoom(m_state.position.room);
    getWorld().getObjectManager().getLara().setAnimation(loader::file::AnimationId::SQUASH_BOULDER, 3561_frame);
    getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
    getWorld().getCameraController().setEyeRotation(-25_deg, 170_deg);
    getWorld().getObjectManager().getLara().m_state.rotation.X = 0_deg;
    getWorld().getObjectManager().getLara().m_state.rotation.Y = m_state.rotation.Y;
    getWorld().getObjectManager().getLara().m_state.rotation.Z = 0_deg;
    getWorld().getObjectManager().getLara().setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
    getWorld().getObjectManager().getLara().setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
    for(int i = 0; i < 15; ++i)
    {
      const auto tmp = getWorld().getObjectManager().getLara().m_state.position.position
                       + core::TRVec{util::rand15s(128_len), -util::rand15s(512_len), util::rand15s(128_len)};
      auto fx = createBloodSplat(getWorld(),
                                 RoomBoundPosition{m_state.position.room, tmp},
                                 2 * m_state.speed,
                                 util::rand15s(22.5_deg) + m_state.rotation.Y);
      getWorld().getObjectManager().registerParticle(fx);
    }
    return;
  }

  if(collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
  {
    enemyPush(collisionInfo, collisionInfo.policies.is_set(CollisionInfo::PolicyFlags::EnableSpaz), true);
  }
  getWorld().getObjectManager().getLara().m_state.health -= 100_hp;
  const auto x = getWorld().getObjectManager().getLara().m_state.position.position.X - m_state.position.position.X;
  const auto y = getWorld().getObjectManager().getLara().m_state.position.position.Y - 350_len
                 - (m_state.position.position.Y - 2 * core::QuarterSectorSize);
  const auto z = getWorld().getObjectManager().getLara().m_state.position.position.Z - m_state.position.position.Z;
  const auto xyz = std::max(2 * core::QuarterSectorSize, sqrt(util::square(x) + util::square(y) + util::square(z)));

  auto fx
    = createBloodSplat(getWorld(),
                       RoomBoundPosition{m_state.position.room,
                                         core::TRVec{x * core::SectorSize / 2 / xyz + m_state.position.position.X,
                                                     y * core::SectorSize / 2 / xyz + m_state.position.position.Y
                                                       - 2 * core::QuarterSectorSize,
                                                     z * core::SectorSize / 2 / xyz + m_state.position.position.Z}},
                       m_state.speed,
                       m_state.rotation.Y);
  getWorld().getObjectManager().registerParticle(fx);
}
