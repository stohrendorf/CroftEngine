#include "swingingblade.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/particle.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <bitset>
#include <gl/renderstate.h>
#include <memory>

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
      getWorld().getObjectManager().getLara().m_state.location.position.X + util::rand15s(128_len),
      getWorld().getObjectManager().getLara().m_state.location.position.Y - util::rand15(745_len),
      getWorld().getObjectManager().getLara().m_state.location.position.Z + util::rand15s(128_len)};
    auto fx = createBloodSplat(getWorld(),
                               Location{m_state.location.room, splatPos},
                               getWorld().getObjectManager().getLara().m_state.speed,
                               getWorld().getObjectManager().getLara().m_state.rotation.Y + util::rand15s(+22_deg));
    getWorld().getObjectManager().registerParticle(fx);
  }

  const auto sector = m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);
  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

  ModelObject::update();
}

void SwingingBlade::collide(CollisionInfo& collisionInfo)
{
  trapCollideWithLara(collisionInfo);
}

void SwingingBlade::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
}

void SwingingBlade::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);
}

SwingingBlade::SwingingBlade(const std::string& name,
                             const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, true}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
