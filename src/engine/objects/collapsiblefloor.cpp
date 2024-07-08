#include "collapsiblefloor.h"

#include "core/id.h"
#include "core/units.h"
#include "engine/heightinfo.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"
#include "serialization/serialization.h"

#include <gl/renderstate.h>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

namespace engine::objects
{
void CollapsibleFloor::update()
{
  if(m_state.current_anim_state == 0_as) // stationary
  {
    if(m_state.location.position.Y - 512_len != getWorld().getObjectManager().getLara().m_state.location.position.Y)
    {
      m_state.triggerState = TriggerState::Inactive;
      deactivate();
      return;
    }
    m_state.goal_anim_state = 1_as;
  }
  else if(m_state.current_anim_state == 1_as) // shaking
  {
    m_state.goal_anim_state = 2_as;
  }
  else if(m_state.current_anim_state == 2_as && m_state.goal_anim_state != 3_as) // falling, not going to settle
  {
    m_state.falling = true;
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
    return;
  }

  const auto sector = m_state.location.updateRoom();
  setCurrentRoom(m_state.location.room);

  m_state.floor
    = HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;

  if(m_state.current_anim_state != 2_as || m_state.location.position.Y < m_state.floor)
    return;

  // settle
  m_state.goal_anim_state = 3_as;
  m_state.location.position.Y = m_state.floor;
  m_state.fallspeed = 0_spd;
  m_state.falling = false;
}

CollapsibleFloor::CollapsibleFloor(const gsl::not_null<world::World*>& world, const Location& location)
    : ModelObject{world, location}
{
}

CollapsibleFloor::CollapsibleFloor(const std::string& name,
                                   const gsl::not_null<world::World*>& world,
                                   const gsl::not_null<const world::Room*>& room,
                                   const loader::file::Item& item,
                                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}

void CollapsibleFloor::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
}

void CollapsibleFloor::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
