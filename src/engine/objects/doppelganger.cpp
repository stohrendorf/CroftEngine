#include "doppelganger.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/heightinfo.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/animation.h"
#include "engine/world/room.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/animationid.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"

#include <cstddef>
#include <exception>
#include <memory>

namespace engine::objects
{
Doppelganger::Doppelganger(const std::string& name,
                           const gsl::not_null<world::World*>& world,
                           const gsl::not_null<const world::Room*>& room,
                           const loader::file::Item& item,
                           const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  const auto& laraModel = world->getWorldGeometry().findAnimatedModelForType(TR1ItemId::Lara);
  getSkeleton()->setAnimation(
    m_state.current_anim_state, gsl::not_null{&laraModel->animations[0]}, laraModel->animations->firstFrame);
}

void Doppelganger::update()
{
  auto& lara = getWorld().getObjectManager().getLara();

  if(m_state.health < core::LaraHealth)
  {
    getWorld().hitLara(10 * (core::LaraHealth - m_state.health));
    m_state.health = core::LaraHealth;
  }

  if(!m_killed)
  {
    m_state.location = lara.m_state.location;
    m_state.location.position = core::TRVec{
      72_sectors - m_state.location.position.X, m_state.location.position.Y, 120_sectors - m_state.location.position.Z};
    m_state.rotation = lara.m_state.rotation - core::TRRotation{0_deg, 180_deg, 0_deg};

    const auto sector = m_state.location.updateRoom();
    setParent(gsl::not_null{getNode()}, m_state.location.room->node);
    m_state.floor
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

    const auto laraSector = lara.m_state.location.moved({}).updateRoom();
    const auto laraFloor
      = HeightInfo::fromFloor(laraSector, lara.m_state.location.position, getWorld().getObjectManager().getObjects()).y;
    getSkeleton()->setAnim(gsl::not_null{lara.getSkeleton()->getAnim()}, lara.getSkeleton()->getFrame());

    if(laraFloor + 1_sectors <= m_state.floor && !lara.m_state.falling)
    {
      m_killed = true;

      getSkeleton()->setAnimation(
        m_state.current_anim_state,
        gsl::not_null{&getWorld().getWorldGeometry().getAnimation(loader::file::AnimationId::SMASH_JUMP)},
        getWorld().getWorldGeometry().getAnimation(loader::file::AnimationId::SMASH_JUMP).firstFrame + 1_frame);
      m_state.goal_anim_state = loader::file::LaraStateId::FreeFall;
      m_state.current_anim_state = loader::file::LaraStateId::FreeFall;
      m_state.fallspeed = 0_spd;
      m_state.falling = true;
      m_state.speed = 0_spd;
      m_state.location.position.Y += 50_len;
    }
  }

  if(m_killed)
  {
    ModelObject::update();
    const auto sector = m_state.location.moved({}).updateRoom();
    const auto hi
      = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects());
    m_state.floor = hi.y;
    getWorld().handleCommandSequence(hi.lastCommandSequenceOrDeath, true);
    if(m_state.floor > m_state.location.position.Y)
      return;

    m_state.location.position.Y = m_state.floor;
    const auto sector2 = m_state.location.moved({}).updateRoom();
    getWorld().handleCommandSequence(
      HeightInfo::fromFloor(sector2, m_state.location.position, getWorld().getObjectManager().getObjects())
        .lastCommandSequenceOrDeath,
      true);
    m_state.fallspeed = 0_spd;
    m_state.falling = false;
    m_state.goal_anim_state = loader::file::LaraStateId::Death;
    m_state.required_anim_state = loader::file::LaraStateId::Death;
  }
  else
  {
    gsl_Assert(getSkeleton()->getBoneCount() == lara.getSkeleton()->getBoneCount());
    for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
    {
      getSkeleton()->setPoseMatrix(i, lara.getSkeleton()->getPoseMatrix(i));
    }

    applyTransform();
  }
}

void Doppelganger::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
  ser(S_NV("killed", m_killed));
}

void Doppelganger::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  ser(S_NV("killed", m_killed));
}
} // namespace engine::objects
