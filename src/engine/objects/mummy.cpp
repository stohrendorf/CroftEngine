#include "mummy.h"

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <memory>

namespace engine::objects
{
Mummy::Mummy(const std::string& name,
             const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel}
{
  for(size_t idx : {11, 12, 13, 14})
    getSkeleton()->setVisible(idx, false);
  getSkeleton()->rebuildMesh();
}

void Mummy::update()
{
  if(m_state.current_anim_state == 1_as)
  {
    auto head = angleFromAtan(
      getWorld().getObjectManager().getLara().m_state.location.position.X - m_state.location.position.X,
      getWorld().getObjectManager().getLara().m_state.location.position.Z - m_state.location.position.Z);
    head = std::clamp(head - m_state.rotation.Y, -90_deg, +90_deg);
    m_headRotation += std::clamp(head - m_headRotation, -5_deg, +5_deg);
    getSkeleton()->patchBone(3, core::TRRotation{0_deg, m_headRotation, 0_deg}.toMatrix());

    if(m_state.isDead() || m_state.touch_bits.any())
    {
      m_state.goal_anim_state = 2_as;
    }
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
    m_state.health = core::DeadHealth;
  }
}

void Mummy::collide(CollisionInfo& info)
{
  collideWithLara(info);
}
} // namespace engine::objects
