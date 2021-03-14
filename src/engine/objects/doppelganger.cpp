#include "doppelganger.h"

#include "engine/particle.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
Doppelganger::Doppelganger(const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const loader::file::Room*>& room,
                           const loader::file::Item& item,
                           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{world, room, item, true, animatedModel}
{
  const auto& laraModel = world->findAnimatedModelForType(TR1ItemId::Lara);
  getSkeleton()->setAnimation(m_state.current_anim_state, laraModel->animations, laraModel->animations->firstFrame);
}

void Doppelganger::update()
{
  auto& lara = getWorld().getObjectManager().getLara();

  if(m_state.health < core::LaraHealth)
  {
    lara.m_state.health -= 10 * (core::LaraHealth - m_state.health);
    m_state.health = core::LaraHealth;
  }

  if(!m_flag)
  {
    const auto twinPos = core::TRVec{72 * core::SectorSize - lara.m_state.position.position.X,
                                     lara.m_state.position.position.Y,
                                     120 * core::SectorSize - lara.m_state.position.position.Z};

    const auto sector = findRealFloorSector(twinPos, &m_state.position.room);
    setParent(getNode(), m_state.position.room->node);
    m_state.floor = HeightInfo::fromCeiling(sector, twinPos, getWorld().getObjectManager().getObjects()).y;

    const auto laraSector = findRealFloorSector(lara.m_state.position.position, lara.m_state.position.room);
    const auto laraHeight
      = HeightInfo::fromFloor(laraSector, lara.m_state.position.position, getWorld().getObjectManager().getObjects()).y;
    getSkeleton()->setAnim(lara.getSkeleton()->getAnim(), lara.getSkeleton()->getFrame());
    m_state.position.position = twinPos;
    m_state.rotation = lara.m_state.rotation - core::TRRotation{0_deg, 180_deg, 0_deg};

    if(laraHeight + core::SectorSize <= m_state.floor && !lara.m_state.falling)
    {
      m_flag = true;

      m_state.goal_anim_state = 9_as;
      m_state.current_anim_state = 9_as;
      getSkeleton()->setAnimation(
        m_state.current_anim_state, &getWorld().findAnimatedModelForType(TR1ItemId::Lara)->animations[32], 481_frame);
      m_state.fallspeed = 0_spd;
      m_state.speed = 0_spd;
      m_state.falling = true;
      m_state.position.position.Y += 50_len;
    }
  }

  if(m_flag)
  {
    ModelObject::update();
    const auto oldPos = m_state.position.position;
    const auto sector = findRealFloorSector(m_state.position.position, m_state.position.room);
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects());
    const auto height = hi.y;
    m_state.floor = height;
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor > m_state.position.position.Y)
      return;

    m_state.position.position.Y = m_state.floor;
    m_state.floor = hi.y;
    const auto sector2 = findRealFloorSector({oldPos.X, hi.y, oldPos.Z}, m_state.position.room);
    const auto hi2
      = HeightInfo::fromFloor(sector2, {oldPos.X, hi.y, oldPos.Z}, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi2.lastCommandSequenceOrDeath, true);
    m_state.fallspeed = 0_spd;
    m_state.goal_anim_state = 8;
    m_state.required_anim_state = 8;
    m_state.falling = false;
  }
}
} // namespace engine::objects
