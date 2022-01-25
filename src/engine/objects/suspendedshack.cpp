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
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::objects
{
void SuspendedShack::update()
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

  ModelObject::update();
}

void SuspendedShack::collide(CollisionInfo& collisionInfo)
{
  collideWithLara(collisionInfo);
}

void SuspendedShack::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  if(ser.loading)
    getSkeleton()->getRenderState().setScissorTest(false);
}

SuspendedShack::SuspendedShack(const std::string& name,
                               const gsl::not_null<world::World*>& world,
                               const gsl::not_null<const world::Room*>& room,
                               const loader::file::Item& item,
                               const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
