#include "boulder.h"

#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"

void engine::objects::RollingBall::update()
{
  if(m_state.triggerState == TriggerState::Active)
  {
    if(m_state.location.position.Y >= m_state.floor)
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
    const auto oldPos = m_state.location.position;
    ModelObject::update();

    auto sector = m_state.location.updateRoom();
    setCurrentRoom(m_state.location.room);
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    m_state.floor = hi.y;
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor - core::QuarterSectorSize <= m_state.location.position.Y)
    {
      m_state.fallspeed = 0_spd;
      m_state.falling = false;
      m_state.location.position.Y = m_state.floor;
    }

    // let's see if we hit a wall, and if that's the case, stop.
    auto testPos = m_state.location.moved(util::pitch(core::SectorSize / 2, m_state.rotation.Y));
    sector = testPos.updateRoom();
    if(HeightInfo::fromFloor(sector, testPos.position, getWorld().getObjectManager().getObjects()).y
       < m_state.location.position.Y)
    {
      m_state.fallspeed = 0_spd;
      m_state.touch_bits.reset();
      m_state.speed = 0_spd;
      m_state.triggerState = TriggerState::Deactivated;
      m_state.location.position.X = oldPos.X;
      m_state.location.position.Y = m_state.floor;
      m_state.location.position.Z = oldPos.Z;
    }
  }
  else if(m_state.triggerState == TriggerState::Deactivated && !m_state.updateActivationTimeout())
  {
    m_state.triggerState = TriggerState::Deactivated;
    m_state.location.position = m_location.position;
    setCurrentRoom(m_location.room);
    getSkeleton()->setAnimation(
      m_state.current_anim_state, getWorld().findAnimatedModelForType(m_state.type)->animations, 0_frame);
    m_state.goal_anim_state = m_state.current_anim_state;
    m_state.required_anim_state = 0_as;
    deactivate();
  }
}

void engine::objects::RollingBall::collide(CollisionInfo& collisionInfo)
{
  auto& lara = getWorld().getObjectManager().getLara();

  if(m_state.triggerState != TriggerState::Active)
  {
    if(m_state.triggerState != TriggerState::Invisible)
      collideWithLara(collisionInfo);

    return;
  }

  if(!isNear(lara, collisionInfo.collisionRadius))
    return;

  if(!testBoneCollision(lara))
    return;

  if(!lara.m_state.falling)
  {
    lara.m_state.is_hit = true;
    if(lara.isDead())
      return;

    lara.m_state.health = core::DeadHealth;
    lara.setCurrentRoom(m_state.location.room);
    lara.setAnimation(loader::file::AnimationId::SQUASH_BOULDER);
    getWorld().getCameraController().setModifier(CameraModifier::FollowCenter);
    getWorld().getCameraController().setEyeRotation(-25_deg, 170_deg);
    lara.m_state.rotation.X = 0_deg;
    lara.m_state.rotation.Y = m_state.rotation.Y;
    lara.m_state.rotation.Z = 0_deg;
    lara.setGoalAnimState(loader::file::LaraStateId::BoulderDeath);
    lara.setCurrentAnimState(loader::file::LaraStateId::BoulderDeath);
    for(int i = 0; i < 15; ++i)
    {
      const auto tmp = lara.m_state.location.position
                       + core::TRVec{util::rand15s(128_len), -util::rand15s(512_len), util::rand15s(128_len)};
      auto fx = createBloodSplat(getWorld(),
                                 Location{m_state.location.room, tmp},
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
  lara.m_state.health -= 100_hp;
  const auto x = lara.m_state.location.position.X - m_state.location.position.X;
  const auto y
    = lara.m_state.location.position.Y - 350_len - (m_state.location.position.Y - 2 * core::QuarterSectorSize);
  const auto z = lara.m_state.location.position.Z - m_state.location.position.Z;
  const auto xyz = std::max(2 * core::QuarterSectorSize, sqrt(util::square(x) + util::square(y) + util::square(z)));

  auto fx = createBloodSplat(
    getWorld(),
    Location{m_state.location.room,
             core::TRVec{x * core::SectorSize / 2 / xyz + m_state.location.position.X,
                         y * core::SectorSize / 2 / xyz + m_state.location.position.Y - 2 * core::QuarterSectorSize,
                         z * core::SectorSize / 2 / xyz + m_state.location.position.Z}},
    m_state.speed,
    m_state.rotation.Y);
  getWorld().getObjectManager().registerParticle(fx);
}

engine::objects::RollingBall::RollingBall(const std::string& name,
                                          const gsl::not_null<world::World*>& world,
                                          const gsl::not_null<const world::Room*>& room,
                                          const loader::file::Item& item,
                                          const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
    , m_location{room, item.position}
{
}
