#include "suspendedshack.h"

#include "engine/world/world.h"
#include "laraobject.h"
#include "serialization/serialization.h"

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
} // namespace engine::objects
