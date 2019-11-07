#include "doppelganger.h"

#include "engine/laranode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
Doppelganger::Doppelganger(const gsl::not_null<Engine*>& engine,
                           const gsl::not_null<const loader::file::Room*>& room,
                           const loader::file::Item& item,
                           const loader::file::SkeletalModelType& animatedModel)
    : ModelItemNode{engine, room, item, true, animatedModel}
{
  const auto& laraModel = engine->findAnimatedModelForType(TR1ItemId::Lara);
  getSkeleton()->setAnimation(m_state, laraModel->animations, laraModel->animations->firstFrame);
}

void Doppelganger::update()
{
  auto& lara = getEngine().getLara();

  if(m_state.health < 1000_hp)
  {
    lara.m_state.health -= 10 * (1000_hp - m_state.health);
    m_state.health = 1000_hp;
  }

  if(!m_flag)
  {
    const auto twinPos = core::TRVec{72 * core::SectorSize - lara.m_state.position.position.X,
                                     lara.m_state.position.position.Y,
                                     120 * core::SectorSize - lara.m_state.position.position.Z};

    const auto sector = loader::file::findRealFloorSector(twinPos, &m_state.position.room);
    setParent(getNode(), m_state.position.room->node);
    m_state.floor = HeightInfo::fromCeiling(sector, twinPos, getEngine().getItemNodes()).y;

    const auto laraSector
      = loader::file::findRealFloorSector(lara.m_state.position.position, lara.m_state.position.room);
    const auto laraHeight
      = HeightInfo::fromFloor(laraSector, lara.m_state.position.position, getEngine().getItemNodes()).y;
    m_state.frame_number = lara.m_state.frame_number;
    m_state.anim = lara.m_state.anim;
    m_state.position.position = twinPos;
    m_state.rotation = lara.m_state.rotation - core::TRRotation{0_deg, 180_deg, 0_deg};

    if(laraHeight + core::SectorSize <= m_state.floor && !lara.m_state.falling)
    {
      m_flag = true;

      m_state.goal_anim_state = 9_as;
      m_state.current_anim_state = 9_as;
      getSkeleton()->setAnimation(
        m_state, &getEngine().findAnimatedModelForType(TR1ItemId::Lara)->animations[32], 481_frame);
      m_state.fallspeed = 0_spd;
      m_state.speed = 0_spd;
      m_state.falling = true;
      m_state.position.position.Y += 50_len;
    }
  }

  if(m_flag)
  {
    ModelItemNode::update();
    const auto oldPos = m_state.position.position;
    const auto sector = loader::file::findRealFloorSector(m_state.position.position, m_state.position.room);
    auto hi = HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getItemNodes());
    const auto height = hi.y;
    m_state.floor = height;
    getEngine().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor > m_state.position.position.Y)
      return;

    m_state.position.position.Y = m_state.floor;
    m_state.floor = hi.y;
    const auto sector2 = loader::file::findRealFloorSector({oldPos.X, hi.y, oldPos.Z}, m_state.position.room);
    const auto hi2 = HeightInfo::fromFloor(sector2, {oldPos.X, hi.y, oldPos.Z}, getEngine().getItemNodes());
    getEngine().handleCommandSequence(hi2.lastCommandSequenceOrDeath, true);
    m_state.fallspeed = 0_spd;
    m_state.goal_anim_state = 8;
    m_state.required_anim_state = 8;
    m_state.falling = false;
  }
}
} // namespace items
} // namespace engine
