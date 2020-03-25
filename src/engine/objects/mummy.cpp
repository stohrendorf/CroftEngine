#include "mummy.h"

#include "engine/script/reflection.h"
#include "laraobject.h"

namespace engine::objects
{
Mummy::Mummy(const gsl::not_null<Engine*>& engine,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject{engine, room, item, true, animatedModel}
{
}

void Mummy::update()
{
  if(m_state.current_anim_state == 1_as)
  {
    auto head = angleFromAtan(
      getEngine().getObjectManager().getLara().m_state.position.position.X - m_state.position.position.X,
      getEngine().getObjectManager().getLara().m_state.position.position.Z - m_state.position.position.Z);
    head = util::clamp(head - m_state.rotation.Y, -90_deg, +90_deg);
    m_headRotation += util::clamp(head - m_headRotation, -5_deg, +5_deg);
    getSkeleton()->patchBone(3, core::TRRotation{0_deg, m_headRotation, 0_deg}.toMatrix());

    if(m_state.health <= 0_hp || m_state.touch_bits.any())
    {
      m_state.goal_anim_state = 2_as;
    }
  }

  ModelObject::update();

  if(m_state.triggerState == TriggerState::Deactivated)
  {
    deactivate();
    m_state.health = -16384_hp;
  }
}

void Mummy::collide(CollisionInfo& info)
{
  if(!isNear(getEngine().getObjectManager().getLara(), info.collisionRadius))
    return;

  if(!testBoneCollision(getEngine().getObjectManager().getLara()))
    return;

  if(!info.policyFlags.is_set(CollisionInfo::PolicyFlags::EnableBaddiePush))
    return;

  enemyPush(info, false, true);
}
} // namespace engine::objects
