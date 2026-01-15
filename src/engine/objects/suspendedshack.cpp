#include "suspendedshack.h"

#include "core/id.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "modelobject.h"
#include "objectstate.h"
#include "serialization/serialization.h" // IWYU pragma: keep

#include <array>
#include <gl/renderstate.h>
#include <gsl-lite/gsl-lite.hpp>
#include <string>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::objects
{
void SuspendedShack::updateLogic()
{
  if(m_state.activationState.isFullyActivated())
  {
    if(m_state.current_anim_state == 0_as)
    {
      m_state.goal_anim_state = 1_as;
    }
    else if(m_state.current_anim_state == 1_as)
    {
      m_state.goal_anim_state = 2_as;
    }
    else if(m_state.current_anim_state == 2_as)
    {
      m_state.goal_anim_state = 3_as;
    }
    m_state.activationState.fullyDeactivate();
  }

  if(m_state.current_anim_state == 4_as)
  {
    getWorld().getMapFlipActivationStates().at(3).fullyActivate();
    getWorld().swapAllRooms();
    kill();
  }

  advanceFrame();
}

void SuspendedShack::collide(CollisionInfo& collisionInfo)
{
  collideWithLara(collisionInfo);
}

void SuspendedShack::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
}

void SuspendedShack::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  getSkeleton()->getRenderState().setScissorTest(false);
}

SuspendedShack::SuspendedShack(const std::string& name,
                               const gsl_lite::not_null<world::World*>& world,
                               const gsl_lite::not_null<const world::Room*>& room,
                               const loader::file::Item& item,
                               const gsl_lite::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects