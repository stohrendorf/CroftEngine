#include "doppelganger.h"

#include "engine/particle.h"
#include "engine/world/animation.h"
#include "engine/world/world.h"
#include "laraobject.h"

namespace engine::objects
{
Doppelganger::Doppelganger(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
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
    m_state.location = lara.m_state.location;
    m_state.location.position = core::TRVec{72 * core::SectorSize - m_state.location.position.X,
                                            m_state.location.position.Y,
                                            120 * core::SectorSize - m_state.location.position.Z};
    m_state.rotation = lara.m_state.rotation - core::TRRotation{0_deg, 180_deg, 0_deg};

    const auto sector = m_state.location.updateRoom();
    setParent(getNode(), m_state.location.room->node);
    m_state.floor
      = HeightInfo::fromCeiling(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

    const auto laraSector = lara.m_state.location.moved({}).updateRoom();
    const auto laraHeight
      = HeightInfo::fromFloor(laraSector, lara.m_state.location.position, getWorld().getObjectManager().getObjects()).y;
    getSkeleton()->setAnim(lara.getSkeleton()->getAnim(), lara.getSkeleton()->getFrame());

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
      m_state.location.position.Y += 50_len;
    }
  }

  if(m_flag)
  {
    ModelObject::update();
    auto oldLocation = m_state.location;
    const auto sector = m_state.location.moved({}).updateRoom();
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    const auto height = hi.y;
    m_state.floor = height;
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor > m_state.location.position.Y)
      return;

    m_state.location.position.Y = m_state.floor;
    m_state.floor = hi.y;
    oldLocation.position.Y = hi.y;
    const auto sector2 = oldLocation.moved({}).updateRoom();
    const auto hi2 = HeightInfo::fromFloor(sector2, oldLocation.position, getWorld().getObjectManager().getObjects());
    getWorld().handleCommandSequence(hi2.lastCommandSequenceOrDeath, true);
    m_state.fallspeed = 0_spd;
    m_state.goal_anim_state = 8;
    m_state.required_anim_state = 8;
    m_state.falling = false;
  }
}
} // namespace engine::objects
